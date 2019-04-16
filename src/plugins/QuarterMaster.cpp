// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "QuarterMaster.h"
#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

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
            return sum + 15;
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING:
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: {
            float CC_to_SB_ratio = 1 - (gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT).build_time /
                                   gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER).build_time);

            if(unit_->build_progress > CC_to_SB_ratio) // We use this number because if it's smaller it will be faster to build a new
                return sum + 15.0f;                    // supply depot than to wait for the command center to finish.

            return sum;
        }
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
        case sc2::UNIT_TYPEID::ZERG_HIVE:
        case sc2::UNIT_TYPEID::ZERG_LAIR:
            return sum + 6.0f;

        case sc2::UNIT_TYPEID::ZERG_EGG: {
            if (unit_->GetPreviousStepOrders().front().ability_id == sc2::ABILITY_ID::TRAIN_OVERLORD)
                return sum + 8.0f;

            return sum;
        }
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

struct CalcDemand {
    float operator()(float sum, const Order& order_) const;
};

float CalcDemand::operator()(float sum, const Order& order_) const {
    return sum + order_.food_required;
}

}  // namespace

void QuarterMaster::OnStep(Builder* builder_) {
    if (gAPI->observer().GetFoodCap() >= 200.0f)
        return;

    float expected_demand = CalcEstimatedDemand(builder_);
    float expected_supply = CalcEstimatedSupply(builder_);

    if (expected_supply > expected_demand || expected_supply >= 200.0f)
        return;

    gHistory.info() << "Request additional supplies: " <<
        expected_demand << " >= " << expected_supply << std::endl;

    builder_->ScheduleSequentialConstruction(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, true);
}

float QuarterMaster::CalcEstimatedDemand(Builder* builder_) {
    // Demand of currently scheduled training
    const auto& training = builder_->GetTrainingOrders();
    float demand = std::accumulate(training.begin(), training.end(), 0.0f, CalcDemand());

    // Assume buildings currently producing will produce the same unit again, and include that in demand
    auto units = gAPI->observer().GetUnits(IsBuilding(), sc2::Unit::Alliance::Self);
    for (auto& unit : units) {
        for (auto& order : unit->GetPreviousStepOrders()) {
            auto constructed_unit_type = gAPI->observer().GetUnitConstructedFromAbility(order.ability_id);
            if (constructed_unit_type == sc2::UNIT_TYPEID::INVALID)
                continue;

            auto constructed_unit_data = gAPI->observer().GetUnitTypeData(constructed_unit_type);
            if (constructed_unit_data.food_provided == 0)
                demand += constructed_unit_data.food_required;
        }
    }

    return gAPI->observer().GetFoodUsed() + demand;
}

float QuarterMaster::CalcEstimatedSupply(Builder* builder_) {
    auto units = gAPI->observer().GetUnits(sc2::Unit::Alliance::Self);
    const auto& non_seq = builder_->GetNonsequentialConstructionOrders();
    const auto& seq = builder_->GetSequentialConstructionOrders();
    const auto& training = builder_->GetTrainingOrders();
    const auto& constructions = gHub->GetConstructions();

    // We need to manually include supply depots that any SCV is on his way to build
    float queued_supply = 0.0f;
    auto scvs = gAPI->observer().GetUnits(IsWorkerWithJob(Worker::Job::building), sc2::Unit::Alliance::Self);
    for (auto& scv : scvs) {
        // Only include supply depots (CC's take too long to build)
        if (scv->GetPreviousStepOrders().empty() ||
            scv->GetPreviousStepOrders().front().ability_id != sc2::ABILITY_ID::BUILD_SUPPLYDEPOT)
            continue;

        // Skip any SCV who's already started his building
        auto itr = std::find_if(constructions.begin(), constructions.end(), [scv](auto& c) { return c.GetScv() == scv; });
        if (itr != constructions.end())
            continue;

        queued_supply += 8.0f;
    }

    return
        queued_supply +
        std::accumulate(units.begin(), units.end(), 0.0f, CalcSupplies()) +
        std::accumulate(non_seq.begin(), non_seq.end(), 0.0f, CalcSupplies()) +
        std::accumulate(seq.begin(), seq.end(), 0.0f, CalcSupplies()) +
        std::accumulate(training.begin(), training.end(), 0.0f, CalcSupplies());
}
