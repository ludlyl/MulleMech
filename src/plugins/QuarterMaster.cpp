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
};

float CalcSupplies::operator()(float sum, const Unit* unit_) const {
    // Even though MulleMech is only able to play terran,
    // it's good that this function is valid for all races if we want to calculate our opponents supply
    sc2::UNIT_TYPEID unit_type = unit_->unit_type;

    /*
     * Tested on 2019-05-01 and this code isn't benifitial (we don't really get supply blocked anyway).
     * This might change in the future though and if we get supply blocked it might be worth trying this out
     * (should probably be generalized for all buildings that take longer to build than depots etc. though)
     *
       float SB_to_CC_buildtime_ratio = 1 - (gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT).build_time /
                                   gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER).build_time);
       if(unit_->build_progress > SB_to_CC_buildtime_ratio)
     *
     * */

    if (unit_->unit_type == sc2::UNIT_TYPEID::ZERG_EGG) {
        if (unit_->GetPreviousStepOrders().front().ability_id == sc2::ABILITY_ID::TRAIN_OVERLORD) {
            unit_type = sc2::UNIT_TYPEID::ZERG_OVERLORD;
        }
    }

    return sum + gAPI->observer().GetUnitTypeData(unit_type).food_provided;
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

    // Assume buildings currently producing will produce the same unit(s) two more times, and include that in demand
    auto units = gAPI->observer().GetUnits(IsBuilding(), sc2::Unit::Alliance::Self);
    for (auto& unit : units) {
        for (auto& order : unit->GetPreviousStepOrders()) {
            auto constructed_unit_type = gAPI->observer().GetUnitConstructedFromAbility(order.ability_id);
            if (constructed_unit_type == sc2::UNIT_TYPEID::INVALID)
                continue;

            auto constructed_unit_data = gAPI->observer().GetUnitTypeData(constructed_unit_type);
            if (constructed_unit_data.food_provided == 0) {
                // Might be more optimal to multiply demand with some (arbitrary) modifier instead of doing * 2 here
                demand += constructed_unit_data.food_required * 2;
            }
        }
    }

    return gAPI->observer().GetFoodUsed() + demand;
}

float QuarterMaster::CalcEstimatedSupply(Builder* builder_) {
    auto units = gAPI->observer().GetUnits(sc2::Unit::Alliance::Self);

    // CountScheduledStructures also counts scvs on their way to build the building
    // As we don't play zerg we do not have to check training orders
    float scheduled_supply_depots = builder_->CountScheduledStructures(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT);
    float supply_from_depot = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT).food_provided;

    return
        scheduled_supply_depots * supply_from_depot +
        std::accumulate(units.begin(), units.end(), 0.0f, CalcSupplies());
}
