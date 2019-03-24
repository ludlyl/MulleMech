// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "blueprints/Blueprint.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "objects/Worker.h"
#include "Builder.h"
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

    sc2::UnitTypeData scv = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SCV);
    int num_of_workers = static_cast<int>(gAPI->observer().GetUnits(IsWorker(), sc2::Unit::Self).size());
    int const max_workers = 70;
    //reserve minerals for scv's
    if (num_of_workers < max_workers)
        m_minerals -= scv.mineral_cost;
    auto it = m_construction_orders.begin();
    // TODO: Fix for mutations and add-ons (currently all orders will be fulfilled on one building)
    while (it != m_construction_orders.end()) {
        if (!Build(&(*it)))
            break;

        it = m_construction_orders.erase(it);
    }

    it = m_training_orders.begin();
    while (it != m_training_orders.end()) {
        if (!Build(&(*it))) {
            ++it;
            continue;
        }
        it = m_training_orders.erase(it);
    }

    // Find new SCVs if construction stopped on any building due to SCV's getting killed off
    ResolveMissingWorkers();
}

void Builder::ScheduleConstruction(sc2::UNIT_TYPEID id_, bool urgent, Unit* unit_) {
    Order order(gAPI->observer().GetUnitTypeData(id_), unit_);

    if (urgent) {
        m_construction_orders.emplace_front(order);
        // Prevent deadlock
        ScheduleRequiredStructures(order, true);
    } else {
        // Prevent deadlock
        ScheduleRequiredStructures(order, false);
        m_construction_orders.emplace_back(std::move(order));
    }
}

void Builder::ScheduleUpgrade(sc2::UPGRADE_ID id_) {
    m_construction_orders.emplace_back(gAPI->observer().GetUpgradeData(id_));
}

void Builder::ScheduleTraining(sc2::UNIT_TYPEID id_, bool urgent, Unit* unit_) {
    auto data = gAPI->observer().GetUnitTypeData(id_);

    if (urgent) {
        m_training_orders.emplace_front(data, unit_);
    } else {
        m_training_orders.emplace_back(data, unit_);
    }
}

void Builder::ScheduleOrders(const std::vector<Order>& orders_, bool urgent) {
    // FIXME (alkurbatov): this call must be more intellectual
    // and able to select a proper queue.
    for (const auto& i : orders_) {
        if (urgent) {
            m_training_orders.emplace_front(i);
        } else {
            m_training_orders.emplace_back(i);
        }
    }
}

const std::list<Order>& Builder::GetConstructionOrders() const {
    return m_construction_orders;
}

const std::list<Order>& Builder::GetTrainingOrders() const {
    return m_training_orders;
}

int64_t Builder::CountScheduledStructures(sc2::UNIT_TYPEID id_) const {
    return std::count_if(
        m_construction_orders.begin(),
        m_construction_orders.end(),
        IsOrdered(id_));
}

int64_t Builder::CountScheduledTrainings(sc2::UNIT_TYPEID id_) const {
    return std::count_if(
        m_training_orders.begin(),
        m_training_orders.end(),
        IsOrdered(id_));
}

bool Builder::Build(Order* order_) {
    if (m_minerals < order_->mineral_cost || m_vespene < order_->vespene_cost)
        return false;

    if (!HasTechRequirements(order_))
        return false;

    if (m_available_food < order_->food_required)
        return false;

    // If the order is to construct a building, we want to make sure a free worker exists before we continue
    // This is needed to avoid continuously performing expensive operations such as calculating building placement (if no free worker exists)
    if (IsBuilding()(order_->unit_type_id)) {
        if (!gHub->FreeWorkerExists()) {
            return false;
        }
    }

    std::shared_ptr<bp::Blueprint> blueprint = bp::Blueprint::Plot(order_->ability_id);

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
        if (gAPI->observer().CountUnitType(unitTypeID) == 0 && CountScheduledStructures(unitTypeID) == 0) {
            ScheduleConstruction(unitTypeID, urgent);
        }
    }
}

bool Builder::HasTechRequirements(Order *order_) const {
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
        if (!construction.GetScv() && building) {
            auto worker = gHub->GetClosestFreeWorker(building->pos);
            if (worker) {
                gHistory.debug() << "Sent new SCV to construct " << UnitTypeToName(building->unit_type) <<
                    "; other one died" << std::endl;
                gAPI->action().Cast(worker, sc2::ABILITY_ID::SMART, building);
                construction.scv = worker;
            }
        }
    }
}
