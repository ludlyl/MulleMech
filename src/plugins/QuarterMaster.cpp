// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "QuarterMaster.h"
#include "Historican.h"
#include "Hub.h"
#include "core/API.h"

#include <numeric>

namespace {

struct CalcSupplies {
    float operator()(float sum, const Unit* unit_) const;

    float operator()(float sum, const Order& order_) const;
};

float CalcSupplies::operator()(float sum, const Unit* unit_) const {
    // Even though MulleMech is only able to play terran,
    // it's good that this function is valid for all races if we want to calculate our opponents supply
    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING:
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
            return sum + 15.0f;

        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
        case sc2::UNIT_TYPEID::ZERG_HIVE:
        case sc2::UNIT_TYPEID::ZERG_LAIR:
            return sum + 6.0f;

        case sc2::UNIT_TYPEID::ZERG_EGG:
            if (unit_->orders.front().ability_id == sc2::ABILITY_ID::TRAIN_OVERLORD)
                return sum + 8.0f;

            return sum;

        case sc2::UNIT_TYPEID::PROTOSS_PYLON:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED:
        case sc2::UNIT_TYPEID::ZERG_OVERLORD:
        case sc2::UNIT_TYPEID::ZERG_OVERLORDCOCOON:
        case sc2::UNIT_TYPEID::ZERG_OVERLORDTRANSPORT:
        case sc2::UNIT_TYPEID::ZERG_OVERSEER:
            return sum + 8.0f;

        default:
            return sum;
    }
}

float CalcSupplies::operator()(float sum, const Order& order_) const {
    switch (order_.ability_id.ToType()) {
        case sc2::ABILITY_ID::BUILD_COMMANDCENTER:
            return sum + 15.0f;

        case sc2::ABILITY_ID::BUILD_SUPPLYDEPOT:
            return sum + 8.0f;

        default:
            return sum;
    }
}

struct CalcConsumption {
    float operator()(float sum, const Order& order_) const;
};

float CalcConsumption::operator()(float sum, const Order& order_) const {
    return sum + order_.food_required;
}

}  // namespace

QuarterMaster::QuarterMaster():
    Plugin(), m_skip_turn(false), m_expected_supply_marginal(1.2f) {
}

void QuarterMaster::OnStep(Builder* builder_) {
    if (m_skip_turn)
        return;

    auto units = gAPI->observer().GetUnits(sc2::Unit::Alliance::Self);
    const std::list<Order>& nonseq_construction_orders = builder_->GetNonsequentialConstructionOrders();
    const std::list<Order>& seq_construction_orders = builder_->GetSequentialConstructionOrders();
    const std::list<Order>& training_orders = builder_->GetTrainingOrders();

    float expected_consumption =
        gAPI->observer().GetFoodUsed()
        + std::accumulate(
            training_orders.begin(),
            training_orders.end(),
            0.0f,
            CalcConsumption());

    float expected_supply =(std::accumulate(units.begin(), units.end(), 0.0f, CalcSupplies())
        + std::accumulate(
            nonseq_construction_orders.begin(),
            nonseq_construction_orders.end(),
            0.0f,
            CalcSupplies())
        + std::accumulate(
            seq_construction_orders.begin(),
            seq_construction_orders.end(),
            0.0f,
            CalcSupplies())
        + std::accumulate(
            training_orders.begin(),
            training_orders.end(),
            0.0f,
            CalcSupplies()))*m_expected_supply_marginal;

    if (expected_supply > expected_consumption || expected_supply >= 200.0f)
        return;

    gHistory.info() << "Request additional supplies: " <<
        expected_consumption << " >= " << expected_supply << std::endl;

    m_skip_turn = true;

    builder_->ScheduleSequentialConstruction(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, true);
}

void QuarterMaster::OnUnitCreated(Unit* unit_) {
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
        m_skip_turn = false;
    }
}
