// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "../Hub.h"
#include "Miner.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "core/Order.h"

#include <sc2api/sc2_typeenums.h>

#include <vector>

namespace {
const int mule_energy_cost = 50;

void SecureMineralsIncome(Builder* builder_) {
    std::vector<Order> orders;
    auto town_halls = gAPI->observer().GetUnits(IsTownHall(), sc2::Unit::Alliance::Self);

    for (const auto& i : town_halls()) {
        if (i->assigned_harvesters >= i->ideal_harvesters)
            continue;

        if (!i->orders.empty())
            continue;

        if (builder_->CountScheduledTrainings(gHub->GetCurrentWorkerType()) > 0)
            continue;

        // FIXME (alkurbatov): We should set an assignee for drones
        // and pick a larva closest to the assignee.
        if (gHub->GetCurrentRace() == sc2::Race::Zerg) {
            orders.emplace_back(gAPI->observer().GetUnitTypeData(
                sc2::UNIT_TYPEID::ZERG_DRONE));
            continue;
        }

        orders.emplace_back(gAPI->observer().GetUnitTypeData(
            gHub->GetCurrentWorkerType()), i);
    }

    builder_->ScheduleOrders(orders);
}

void SecureVespeneIncome() {
    auto refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self);
    Units workers = gAPI->observer().GetUnits(IsGasWorker(), sc2::Unit::Alliance::Self);
    for (const auto& i : refineries()) {
     
       if (i->assigned_harvesters == i->ideal_harvesters)
            continue;
       else if (i->assigned_harvesters > i->ideal_harvesters) { // Makes sure that we never have more than 3 workers on gas.
           for (const auto& j : workers()) {
               if (i->tag == j->orders.front().target_unit_tag) {
                   auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
                       sc2::Unit::Alliance::Neutral);
                   const sc2::Unit* mineral_target = units.GetClosestUnit(
                       gAPI->observer().StartingLocation());
                   if (!mineral_target)
                       return;

                   gAPI->action().Cast(*j, sc2::ABILITY_ID::SMART, *mineral_target); // If to many workers on gas -> put one to mine minerals
                   break;
               }
           }
           continue;
       }

       gHub->AssignVespeneHarvester(*i);
    }
}

void CallDownMULE() {
    auto orbitals = gAPI->observer().GetUnits(
        IsUnit(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND), sc2::Unit::Alliance::Self);

    if (orbitals().empty())
        return;

    auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
        sc2::Unit::Alliance::Neutral);

    for (const auto& i : orbitals()) {
        if (i->energy < mule_energy_cost)
            continue;

        const sc2::Unit* mineral_target = units.GetClosestUnit(i->pos);
        if (!mineral_target)
            continue;

        gAPI->action().Cast(*i, sc2::ABILITY_ID::EFFECT_CALLDOWNMULE, *mineral_target);
    }
}

}  // namespace

void Miner::OnStep(Builder* builder_) {
    SecureMineralsIncome(builder_);
    SecureVespeneIncome();

    if (gHub->GetCurrentRace() == sc2::Race::Terran)
        CallDownMULE();
}

void Miner::OnUnitCreated(const sc2::Unit* unit_) {
    if (!IsTownHall()(*unit_))
        return;

    auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
        sc2::Unit::Alliance::Neutral);

    const sc2::Unit* mineral_target = units.GetClosestUnit(unit_->pos);
    if (!mineral_target)
        return;

    gAPI->action().Cast(*unit_, sc2::ABILITY_ID::RALLY_WORKERS, *mineral_target);
}

void Miner::OnUnitIdle(const sc2::Unit* unit_, Builder*) {
    auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
        sc2::Unit::Alliance::Neutral);

    if (gBrain->planner().IsUnitReserved(unit_))
        return;

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::TERRAN_MULE:
        case sc2::UNIT_TYPEID::ZERG_DRONE: {
            const sc2::Unit* mineral_target = units.GetClosestUnit(
                gAPI->observer().StartingLocation());
            if (!mineral_target)
                return;

            gAPI->action().Cast(*unit_, sc2::ABILITY_ID::SMART, *mineral_target);
            break;
        }

        default:
            break;
    }
}
