// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "../Hub.h"
#include "Miner.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "core/Order.h"
#include "core/Timer.h"

#include <sc2api/sc2_typeenums.h>

#include <map>
#include <vector>

namespace {
const int mule_energy_cost = 50;

int NeededWorkers(const std::shared_ptr<Expansion>& expansion) {
    auto gatherStructures = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::Or, {IsCommandCenter(),
        IsRefinery()}), sc2::Unit::Alliance::Self);

    int needed = 0;
    for (auto& structure : gatherStructures) {
        if (sc2::Distance2D(expansion->town_hall_location, structure->pos) < 10.0f)
            needed += structure->ideal_harvesters;
    }
    return needed;
}

Unit* GetMovableWorker(const Units& workers) {
    for (auto& worker : workers) {
        if (worker->AsWorker()->GetJob() == GATHERING_MINERALS)
            return worker;
    }
    return nullptr;
}

void SecureMineralsIncome(Builder* builder_) {
    std::vector<Order> orders;
    auto command_centers = gAPI->observer().GetUnits(IsCommandCenter(), sc2::Unit::Alliance::Self);
    auto refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self);
    auto num_workers = static_cast<int>(gAPI->observer().GetUnits(IsWorker(), sc2::Unit::Alliance::Self).size());
    int optimal_workers = 0;

    // Calculate Optimal Workers
    for (auto& cc : command_centers)
        optimal_workers += cc->ideal_harvesters;
    for (auto& refinery : refineries)
        optimal_workers += refinery->ideal_harvesters;
    optimal_workers = static_cast<int>(std::ceil(optimal_workers * 1.5f)); // Assume ~50% overproduction

    if (num_workers >= optimal_workers)
        return;

    // Schedule training at our Command Centers
    for (auto& cc : command_centers) {
        if (cc->build_progress != 1.0f)
            continue;

        if (!cc->orders.empty())
            continue;

        if (builder_->CountScheduledTrainings(gHub->GetCurrentWorkerType()) > 0)
            continue;

        orders.emplace_back(gAPI->observer().GetUnitTypeData(gHub->GetCurrentWorkerType()), cc);
    }

    if (orders.empty())
        return;

    // TODO: Might not always want scv production to be "urgent/prioritized".
    //  Either make the logic behind this more advanced or add two levels of "urgency" to Builder::ScheduleOrder(s)
    builder_->ScheduleOrders(orders, true);
}


void SecureVespeneIncome() {
    auto refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self);
    Units workers = gAPI->observer().GetUnits(IsGasWorker(), sc2::Unit::Alliance::Self);
    for (const auto& i : refineries) {
     
       if (i->assigned_harvesters == i->ideal_harvesters)
            continue;
       // Makes sure that we never have more than 3 workers on gas.
       else if (i->assigned_harvesters > i->ideal_harvesters) { 
           for (const auto& j : workers) {
               if (i->tag == j->orders.front().target_unit_tag) {
                   auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
                       sc2::Unit::Alliance::Neutral);
                   auto mineral_target = units.GetClosestUnit(i->pos);
                   if (!mineral_target)
                       return;

                   // If to many workers on gas -> put one to mine minerals
                   gAPI->action().Cast(j, sc2::ABILITY_ID::SMART, mineral_target); 
                   break;
               }
           }
           continue;
       }

       gHub->AssignVespeneHarvester(i);
    }
}

void CallDownMULE() {
    auto orbitals = gAPI->observer().GetUnits(
        IsUnit(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND), sc2::Unit::Alliance::Self);

    if (orbitals.empty())
        return;

    auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
        sc2::Unit::Alliance::Neutral);

    for (const auto& i : orbitals) {
        if (i->energy < mule_energy_cost)
            continue;

        auto mineral_target = units.GetClosestUnit(i->pos);
        if (!mineral_target)
            continue;

        gAPI->action().Cast(i, sc2::ABILITY_ID::EFFECT_CALLDOWNMULE, mineral_target);
    }
}

}  // namespace

void Miner::OnStep(Builder* builder_) {
    if (gAPI->observer().GetGameLoop() % 300 == 0) // no need to attempt balance too often (every ~12 seconds)
        BalanceWorkers();
    SecureMineralsIncome(builder_);
    SecureVespeneIncome();

    if (gHub->GetCurrentRace() == sc2::Race::Terran)
        CallDownMULE();
}

void Miner::OnUnitCreated(Unit* unit_) {
    if (IsTownHall()(*unit_)) {
        // Put our CommandCenter's rally point
        auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
            sc2::Unit::Alliance::Neutral);

        auto mineral_target = units.GetClosestUnit(unit_->pos);
        if (!mineral_target)
            return;

        gAPI->action().Cast(unit_, sc2::ABILITY_ID::RALLY_WORKERS, mineral_target);
    } else if (IsWorker()(*unit_)) {
        // Record the workers we have and where they work
        auto ccs = gAPI->observer().GetUnits(IsTownHall(), sc2::Unit::Alliance::Self);
        auto home = ccs.GetClosestUnit(unit_->pos);
        if (home) {
            auto expo = gHub->GetClosestExpansion(home->pos);
            m_expansionWorkers[expo].push_back(unit_);
            unit_->AsWorker()->SetHomeBase(std::move(expo));
        }
    }
}

void Miner::OnUnitDestroyed(Unit* unit_, Builder*) {
    if (IsTownHall()(*unit_)) {
        // A Town Hall died => Reassing workers
        auto expo = gHub->GetClosestExpansion(unit_->pos);

        SplitWorkersOf(expo);

        auto itr = m_expansionWorkers.find(expo);
        if (itr != m_expansionWorkers.end())
            m_expansionWorkers.erase(itr);
    } else if (IsWorker()(*unit_)) {
        // A worker died => Update our status
        auto itr = m_expansionWorkers.find(unit_->AsWorker()->GetHomeBase());
        if (itr != m_expansionWorkers.end())
            itr->second.remove(unit_);
    }
}

void Miner::OnUnitIdle(Unit* unit_, Builder*) {
    if (gBrain->planner().IsUnitReserved(unit_))
        return;

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE: {
            // Send idle worker back to its Home Base
            unit_->AsWorker()->Mine();
            break;
        }
        case sc2::UNIT_TYPEID::TERRAN_MULE: {
            // Send MULE to closest mineral patch of our Starting Location on idle
            // TODO: Maybe send it to nearest mineral patch of a base belonging to us?
            auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(), sc2::Unit::Alliance::Neutral);
            auto mineral_target = units.GetClosestUnit(gAPI->observer().StartingLocation());
            if (!mineral_target)
                return;

            gAPI->action().Cast(unit_, sc2::ABILITY_ID::SMART, mineral_target);
            break;
        }
        default:
            break;
    }
}

void Miner::SplitWorkersOf(const std::shared_ptr<Expansion>& expansion) {
    // Assumption: Other bases are already balanced or closed to balanced
    // Then, if we distribute these workers evenly our bases should still be balanced

    auto itr = m_expansionWorkers.find(expansion);
    if (itr == m_expansionWorkers.end())
        return;

    // Only send to other expansions
    std::vector<std::shared_ptr<Expansion>> otherExpansions;
    otherExpansions.reserve(m_expansionWorkers.size());
    for (auto& pair : m_expansionWorkers) {
        if (pair.first != expansion)
            otherExpansions.push_back(pair.first);
    }

    if (otherExpansions.empty())
        return;

    // Send workers in round-robin fashion
    auto roundRobinItr = otherExpansions.begin();
    for (auto& worker : itr->second) {
        worker->AsWorker()->SetHomeBase(*roundRobinItr);
        worker->AsWorker()->Mine();
        m_expansionWorkers[*roundRobinItr].push_back(worker);
        if (++roundRobinItr == otherExpansions.end())
            roundRobinItr = otherExpansions.begin();
    }
}

void Miner::BalanceWorkers() {
    if (m_expansionWorkers.size() <= 1)
        return;

    Timer timer;
    timer.Start();

    // Build a map of how many workers above their ideal each expansion has
    std::multimap<int, std::shared_ptr<Expansion>> sortedExpansions; // note: maps are ordered by key

    for (auto& pair : m_expansionWorkers) {
        int over = static_cast<int>(pair.second.size()) - NeededWorkers(pair.first);
        sortedExpansions.emplace(std::make_pair(std::max(0, over), pair.first));
    }

    // If ideal difference is >= 4 of top and bottom guy => balance workers
    int diff = sortedExpansions.rbegin()->first - sortedExpansions.begin()->first;
    if (diff >= 4) {
        int move = static_cast<int>(std::ceil(diff / 2.0f));
        int moved = move;
        while (move--) {
            if (auto worker = GetMovableWorker(m_expansionWorkers[sortedExpansions.rbegin()->second])) {
                worker->AsWorker()->SetHomeBase(sortedExpansions.begin()->second);
                worker->AsWorker()->Mine();
                m_expansionWorkers[sortedExpansions.rbegin()->second].remove(worker);
                m_expansionWorkers[sortedExpansions.begin()->second].push_back(worker);
            }
        }

        float time = timer.Finish();
        gHistory.debug(LogChannel::economy) << "Moved " << moved << " workers (" << time << " ms spent). Over Capacity Was: " << std::endl;
#ifdef DEBUG
        std::string str = "    ";
        for (auto& exp : sortedExpansions) {
            int numWorkers = static_cast<int>(m_expansionWorkers[exp.second].size());
            str += std::to_string(exp.first) + " (now: " + std::to_string(numWorkers) + "/" +
                std::to_string(NeededWorkers(exp.second)) + ")    ";
        }
        gHistory.debug(LogChannel::economy) << str << std::endl;
#endif
    }
}
