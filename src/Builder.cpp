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

#include <algorithm>
#include <memory>

Builder::Builder(): m_minerals(0), m_vespene(0), m_available_food(0.0f) {
}

void Builder::OnStep() {
    m_minerals = gAPI->observer().GetMinerals();
    m_vespene = gAPI->observer().GetVespene();

    m_available_food = gAPI->observer().GetAvailableFood();

    // Find new SCVs if construction stopped on any building due to SCV's getting killed off
    ResolveMissingWorkers();

    bool resources_needed_for_nonseq_order = false;
    auto nonseq_order_it = m_nonsequential_construction_orders.begin();
    while (nonseq_order_it != m_nonsequential_construction_orders.end()) {
        if (AreNoneResourceRequirementsFulfilled(&(*nonseq_order_it))) {
            if (Build(&(*nonseq_order_it))) {
                nonseq_order_it = m_nonsequential_construction_orders.erase(nonseq_order_it);
                continue;
            }
            resources_needed_for_nonseq_order = true;
        }
        ++nonseq_order_it;
    }

    // I.e. we don't have to save money for nonsequential construction order
    // TODO: This should be made more advanced to distinguish between minerals/gas
    //  (currently if we have a ton of minerals and just need to save gas for e.g. an upgrade the minerals can't be used in the meanwhile)
    if (!resources_needed_for_nonseq_order) {
        auto it = m_sequential_construction_orders.begin();
        while (it != m_sequential_construction_orders.end()) {
            if (!Build(&(*it)))
                break;

            it = m_sequential_construction_orders.erase(it);
        }

        it = m_training_orders.begin();
        while (it != m_training_orders.end()) {
            if (!Build(&(*it))) {
                ++it;
                continue;
            }
            it = m_training_orders.erase(it);
        }
    }
}

void Builder::ScheduleNonsequentialConstruction(sc2::UNIT_TYPEID id_, Unit *unit_) {
    Order order(gAPI->observer().GetUnitTypeData(id_), unit_);
    m_nonsequential_construction_orders.push_back(std::move(order));
}

void Builder::ScheduleSequentialConstruction(sc2::UNIT_TYPEID id_, bool urgent, Unit *unit_) {
    Order order(gAPI->observer().GetUnitTypeData(id_), unit_);

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

void Builder::ScheduleConstructionInRecommendedQueue(sc2::UNIT_TYPEID id_, bool urgent, Unit *unit_) {
    if (IsAddon()(id_)) {
        ScheduleNonsequentialConstruction(id_, unit_);
        return;
    }

    switch(id_) {
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
            ScheduleNonsequentialConstruction(id_, unit_);
            break;

        default:
            ScheduleSequentialConstruction(id_, urgent, unit_);
    }
}

void Builder::ScheduleUpgrade(sc2::UPGRADE_ID id_) {
    m_nonsequential_construction_orders.emplace_back(gAPI->observer().GetUpgradeData(id_));
}

void Builder::ScheduleTraining(sc2::UNIT_TYPEID id_, bool urgent, Unit* unit_) {
    auto data = gAPI->observer().GetUnitTypeData(id_);

    if (urgent) {
        m_training_orders.emplace_front(data, unit_);
    } else {
        m_training_orders.emplace_back(data, unit_);
    }
}

void Builder::ScheduleTrainingOrders(const std::vector<Order>& orders_, bool urgent) {
    for (const auto& i : orders_) {
        if (urgent) {
            m_training_orders.emplace_front(i);
        } else {
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

int64_t Builder::CountScheduledStructures(sc2::UNIT_TYPEID id_) const {
    return std::count_if(
            m_nonsequential_construction_orders.begin(),
            m_nonsequential_construction_orders.end(),
            IsOrdered(id_)) +
           std::count_if(
            m_sequential_construction_orders.begin(),
            m_sequential_construction_orders.end(),
            IsOrdered(id_));
}

int64_t Builder::CountScheduledTrainings(sc2::UNIT_TYPEID id_) const {
    return std::count_if(
        m_training_orders.begin(),
        m_training_orders.end(),
        IsOrdered(id_));
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

void Builder::ScheduleRequiredStructures(const Order &order_, bool urgent) {
    for (sc2::UnitTypeID unitTypeID : order_.tech_requirements) {
        if (gAPI->observer().CountUnitType(unitTypeID, true) == 0 && CountScheduledStructures(unitTypeID) == 0) {
            ScheduleSequentialConstruction(unitTypeID, urgent);
        }
    }
}

bool Builder::HasTechRequirements(const Order *order_) const {
    for (sc2::UnitTypeID unitTypeID : order_->tech_requirements) {
        if (gAPI->observer().CountUnitType(unitTypeID) == 0) {
            return false;
        }
    }
    return true;
}

void Builder::ResolveMissingWorkers() {
    // Find any unfinished buildings that lacks SCV's working on them
    auto& constructions = gHub->GetConstructions();

    for (auto& construction : constructions) {
        auto building = construction.GetBuilding();
        if (!construction.GetScvIfAlive() && building) {
            auto worker = GetClosestFreeWorker(building->pos);
            if (worker) {
                gHistory.debug() << "Sent new SCV to construct " << UnitTypeToName(building->unit_type) <<
                    "; other one died" << std::endl;
                worker->Build(building);
                construction.scv = worker;
            }
        }
    }
}
