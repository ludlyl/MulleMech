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
}

void Builder::ScheduleConstruction(sc2::UNIT_TYPEID id_, bool urgent, const sc2::Unit* unit_) {
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

void Builder::ScheduleTraining(sc2::UNIT_TYPEID id_, bool urgent, const sc2::Unit* unit_) {
    auto data = gAPI->observer().GetUnitTypeData(id_);

    if (urgent) {
        m_training_orders.emplace_front(data, unit_);
        return;
    }

    m_training_orders.emplace_back(data, unit_);
}

void Builder::ScheduleOrders(const std::vector<Order>& orders_) {
    // FIXME (alkurbatov): this call must be more intellectual
    // and able to select a proper queue.
    for (const auto& i : orders_)
        m_training_orders.emplace_back(i);
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

    std::shared_ptr<Blueprint> blueprint = Blueprint::Plot(order_->ability_id);



    // As this is needed for buildings to work a temporary solution of just checking if food_required == 0
    // to disable the check for all units (it's the Units that seem to be problematic for terran)
    // this has the effect that units that are assigned to a specific structure will fail "silently"
    // (this function, Build, will return true) if they can't be built.

    if (!HasTechRequirements(order_))
        return false;

    if (m_available_food < order_->food_required)
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
