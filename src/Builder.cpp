// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Builder.h"
#include "blueprints/Blueprint.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "objects/Worker.h"
#include "Historican.h"
#include "Hub.h"
#include "BuildingPlacer.h"

#include <algorithm>
#include <memory>

Builder::Builder(): m_minerals(0), m_vespene(0), m_available_food(0.0f) {
}

void Builder::OnStep() {
    m_minerals = gAPI->observer().GetMinerals();
    m_vespene = gAPI->observer().GetVespene();
    m_available_food = gAPI->observer().GetAvailableFood();

    int max_minerals_needed = 0;
    int max_vespene_needed = 0;
    auto it = m_nonsequential_construction_orders.begin();
    while (it != m_nonsequential_construction_orders.end() && m_minerals >= MinimumUnitMineralCost) {
        // Clear out the order if the order has an assignee and that assignee has died
        if (it->assignee && !it->assignee->is_alive)
            it = m_training_orders.erase(it);

        if (!AreNoneResourceRequirementsFulfilled(&(*it))) {
            ++it;
            continue;
        }

        if (Build(&(*it))) {
            it = m_nonsequential_construction_orders.erase(it);
        } else {
            // We want to save enough resources to afford the most expensive thing we can build in the queue
            max_minerals_needed = std::max(max_minerals_needed, it->mineral_cost);
            max_vespene_needed = std::max(max_vespene_needed, it->vespene_cost);
            ++it;
        }
    }

    m_minerals = std::max(0, m_minerals - max_minerals_needed);
    m_vespene = std::max(0, m_vespene - max_vespene_needed);

    it = m_sequential_construction_orders.begin();
    while (it != m_sequential_construction_orders.end() && m_minerals >= MinimumUnitMineralCost) {
        // Clear out the order if the order has an assignee and that assignee has died
        if (it->assignee && !it->assignee->is_alive)
            it = m_training_orders.erase(it);

        if (!Build(&(*it)))
            break;

        it = m_sequential_construction_orders.erase(it);
    }

    bool reserved = false;
    it = m_training_orders.begin();
    while (it != m_training_orders.end() && m_minerals >= MinimumUnitMineralCost) {
        // Clear out the order if the order has an assignee and that assignee has died
        if (it->assignee && !it->assignee->is_alive)
            it = m_training_orders.erase(it);

        if (!AreNoneResourceRequirementsFulfilled(&*it)) {
            ++it;
            continue;
        }

        if (Build(&(*it))) {
            it = m_training_orders.erase(it);
        } else {
            // Reserve resources for first non-buildable unit so the queue has some fairness to it
            if (!reserved) {
                m_minerals = std::max(0, m_minerals - it->mineral_cost);
                m_vespene = std::max(0, m_vespene - it->vespene_cost);
                reserved = true;
            }
            ++it;
        }
    }
}

void Builder::OnUnitCreated(Unit* unit_) {
    // build_progress has to be checked as this function is called for the town hall at the start of the game
    if (unit_->build_progress < 1.0f && IsBuilding()(*unit_) && !IsAddon()(*unit_)) {
        // This should only ever return one worker
        // Checking IsWorkerWithJob is actually unnecessary and removing it would be a small performance improvement
        // (but it's there to make sure the jobs are set correctly and to help assert if this isn't the case)
        // Note: target tag is only set for refineries (and refineries doesn't have a pos, thereby the ugly solution...)
        Units building_workers = gAPI->observer().GetUnits([&unit_](auto& unit) {
            if (IsWorkerWithJob(Worker::Job::building)(unit) && !unit.orders.empty() &&
                unit.orders.front().ability_id == unit_->GetTypeData()->ability_id) {
                if (IsRefinery()(*unit_))
                    return unit.orders.front().target_unit_tag ==
                           gAPI->observer().GetUnits(IsGeyser(), sc2::Unit::Alliance::Neutral).GetClosestUnit(unit_->pos)->tag;
                else
                    return unit.orders.front().target_pos.x == unit_->pos.x &&
                           unit.orders.front().target_pos.y == unit_->pos.y;
            }
            return false;
        }, sc2::Unit::Alliance::Self);

        assert(building_workers.size() == 1 && "More or less than 1 worker was found building a new construction");
        auto worker = building_workers.front()->AsWorker();
        assert(worker->construction && "Worker set as builder but does not have a construction");
        worker->construction->building = unit_;
    }
}

void Builder::OnUnitIdle(Unit* unit_) {
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_SCV) {
        auto worker = unit_->AsWorker();
        auto& construction = worker->construction;
        if (worker->GetJob() == Worker::Job::building && construction) {
            // Construction has finished
            if (construction->building && construction->building->build_progress >= 1.f) {
                worker->construction = nullptr;
            // Building was destroyed while being constructed
            } else if (construction->building && !construction->building->is_alive) {
                worker->construction = nullptr;
            // SCV failed to start construction
            } else if (construction->building == nullptr) {
                gBuildingPlacer->FreeReservedBuildingSpace(construction->position,
                                                           construction->building_type,
                                                           construction->includes_add_on_space);
                ScheduleConstructionInRecommendedQueue(construction->building_type, true);
            }
            worker->SetAsUnemployed();
        }
    }
}

void Builder::OnUnitDestroyed(Unit* unit_) {
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_SCV) {
        auto worker = unit_->AsWorker();
        auto& construction = worker->construction;
        if (worker->GetJob() == Worker::Job::building && construction) {
            // Worker got killed while constructing a building
            if (construction->building && construction->building->build_progress < 1.f) {
                auto new_worker = GetClosestFreeWorker(construction->building->pos);
                if (new_worker) {
                    new_worker->Build(construction->building);
                    new_worker->construction = std::move(worker->construction);
                    worker->construction = nullptr;

                    gHistory.debug() << "Sent new SCV to construct "
                                     << UnitTypeToName((new_worker->construction->building->unit_type))
                                     << "; other one died" << std::endl;
                }

            // Worker got killed before starting the construction
            } else if (worker->construction->building == nullptr) {
                gBuildingPlacer->FreeReservedBuildingSpace(construction->position,
                                                           construction->building_type,
                                                           construction->includes_add_on_space);
                ScheduleConstructionInRecommendedQueue(construction->building_type, true);
            }
        }
    }
}

void Builder::ScheduleNonsequentialConstruction(sc2::UNIT_TYPEID id_, Unit *assignee_) {
    Order order(*gAPI->observer().GetUnitTypeData(id_), assignee_);
    m_nonsequential_construction_orders.push_back(std::move(order));
}

void Builder::ScheduleSequentialConstruction(sc2::UNIT_TYPEID id_, bool urgent, Unit *assignee_) {
    Order order(*gAPI->observer().GetUnitTypeData(id_), assignee_);

    if (urgent) {
        m_sequential_construction_orders.emplace_front(order);
        // Prevent deadlock
        ScheduleRequiredStructures(order, true);
    } else {
        // Prevent deadlock
        ScheduleRequiredStructures(order, false);
        m_sequential_construction_orders.emplace_back(std::move(order));
    }
}

void Builder::ScheduleConstructionInRecommendedQueue(sc2::UNIT_TYPEID id_, bool urgent, Unit *assignee_) {
    if (IsAddon()(id_)) {
        ScheduleNonsequentialConstruction(id_, assignee_);
        return;
    }

    switch(id_) {
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
            ScheduleNonsequentialConstruction(id_, assignee_);
            break;

        default:
            ScheduleSequentialConstruction(id_, urgent, assignee_);
    }
}

void Builder::ScheduleUpgrade(sc2::UPGRADE_ID id_) {
    m_nonsequential_construction_orders.emplace_back(gAPI->observer().GetUpgradeData(id_));
}

void Builder::ScheduleTraining(sc2::UNIT_TYPEID id_, bool urgent, Unit* assignee_) {
    if (IsBuilding()(id_)) {
        assert(false && "Tried to schedule building in training orders queue");
    }

    auto data = gAPI->observer().GetUnitTypeData(id_);

    if (urgent) {
        // Always keep SCVs first in training orders
        auto itr = m_training_orders.begin();
        while (itr != m_training_orders.end() && itr->unit_type_id == sc2::UNIT_TYPEID::TERRAN_SCV)
            ++itr;
        m_training_orders.emplace(itr, *data, assignee_);
    } else {
        m_training_orders.emplace_back(*data, assignee_);
    }
}

void Builder::ScheduleTrainingOrders(const std::vector<Order>& orders_, bool urgent) {
    for (const auto& i : orders_) {
        if (IsBuilding()(i.unit_type_id)) {
            assert(false && "Tried to schedule building in training orders queue");
        }

        if (urgent) {
            // Always keep SCVs first in training orders
            auto itr = m_training_orders.begin();
            while (itr != m_training_orders.end() && itr->unit_type_id == sc2::UNIT_TYPEID::TERRAN_SCV)
                ++itr;
            m_training_orders.emplace(itr, i);
        }
        else {
            m_training_orders.emplace_back(i);
        }
    }
}

const std::list<Order>& Builder::GetNonsequentialConstructionOrders() const {
    return m_nonsequential_construction_orders;
}

const std::list<Order>& Builder::GetSequentialConstructionOrders() const {
    return m_sequential_construction_orders;
}

const std::list<Order>& Builder::GetTrainingOrders() const {
    return m_training_orders;
}

int Builder::CountScheduledStructures(sc2::UNIT_TYPEID id_) const {
    auto non_seq_count = std::count_if(
            m_nonsequential_construction_orders.begin(),
            m_nonsequential_construction_orders.end(),
            IsOrdered(id_));

    auto seq_count = std::count_if(
            m_sequential_construction_orders.begin(),
            m_sequential_construction_orders.end(),
            IsOrdered(id_));

    auto unstarted_order_count = gAPI->observer().GetUnits(IsWorkerWithUnstartedConstructionOrderFor(id_),
                                                           sc2::Unit::Alliance::Self).size();

    return static_cast<int>(static_cast<size_t>(non_seq_count + seq_count) + unstarted_order_count);
}

int Builder::CountScheduledTrainings(sc2::UNIT_TYPEID id_) const {
    return static_cast<int>( std::count_if(
            m_training_orders.begin(),
            m_training_orders.end(),
            IsOrdered(id_)));
}

bool Builder::AreNoneResourceRequirementsFulfilled(Order* order_, std::shared_ptr<bp::Blueprint> blueprint) {
    if (!HasTechRequirements(order_))
        return false;

    if (order_->food_required > 0 && m_available_food < order_->food_required)
        return false;

    if (!blueprint) {
        blueprint = bp::Blueprint::Plot(order_->ability_id);
    }

    // This will/should check if a free worker exists if that is necessary to fulfill the order
    // (so we don't need to check that separately)
    return blueprint->CanBeBuilt(order_);
}

bool Builder::Build(Order* order_) {
    if (m_minerals < order_->mineral_cost || m_vespene < order_->vespene_cost)
        return false;

    // Why does Plot return a shared ptr? Seems pretty pointless
    std::shared_ptr<bp::Blueprint> blueprint = bp::Blueprint::Plot(order_->ability_id);

    if (!AreNoneResourceRequirementsFulfilled(order_, blueprint))
        return false;

    if (!blueprint->Build(order_))
        return false;

    m_minerals -= order_->mineral_cost;
    m_vespene -= order_->vespene_cost;
    m_available_food -= order_->food_required;

    gHistory.info() << "Gave order to start building a " << order_->name << std::endl;
    return true;
}

void Builder::ScheduleRequiredStructures(const Order& order_, bool urgent) {
    for (sc2::UnitTypeID unitTypeID : order_.structure_tech_requirements) {
        if (gAPI->observer().CountUnitType(unitTypeID, true) == 0 && CountScheduledStructures(unitTypeID) == 0) {
            ScheduleSequentialConstruction(unitTypeID, urgent);
        }
    }
}

bool Builder::HasTechRequirements(const Order* order_) const {
    for (sc2::UnitTypeID unitTypeID : order_->structure_tech_requirements) {
        if (gAPI->observer().CountUnitType(unitTypeID) == 0) {
            return false;
        }
    }
    if (order_->upgrade_tech_requirement != sc2::UPGRADE_ID::INVALID &&
        !gAPI->observer().HasUpgrade(order_->upgrade_tech_requirement)) {
        return false;
    }
    return true;
}
