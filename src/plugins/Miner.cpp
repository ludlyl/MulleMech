// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Miner.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Order.h"
#include "core/Timer.h"
#include "Reasoner.h"

#include <sc2api/sc2_typeenums.h>

#include <map>
#include <numeric>
#include <vector>

namespace {
constexpr float maximum_resource_distance = 10.0f;  // Resources further than this => doesn't belong to this base
constexpr int steps_between_balance = 20;           // How often we recalculate SCV balance
constexpr int req_imbalance_to_transfer = 2;        // How many SCVs imbalance we must have before transferring any
constexpr int maximum_workers = 70;                 // Never go above this number of workers

// Counts both for town halls and refineries
int IdealWorkerCount(const std::shared_ptr<Expansion>& expansion) {
    auto gatherStructures = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::Or, {IsTownHall(),
        IsRefinery()}), sc2::Unit::Alliance::Self);

    int needed = 0;
    for (auto& structure : gatherStructures) {
        if (sc2::Distance2D(expansion->town_hall_location, structure->pos) < maximum_resource_distance)
            needed += structure->ideal_harvesters;
    }
    return needed;
}

void SecureMineralsIncome(Builder* builder_) {
    // Cut SCV production on the following playstyles
    switch (gReasoner->GetPlayStyle()) {
        case PlayStyle::all_in:
        case PlayStyle::very_defensive:
            return;
        default:
            break;
    }

    std::vector<Order> orders;
    auto command_centers = gAPI->observer().GetUnits(IsTownHall(), sc2::Unit::Alliance::Self);
    auto refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self);
    auto num_workers = static_cast<int>(gAPI->observer().GetUnits(IsWorker(), sc2::Unit::Alliance::Self).size());
    int optimal_workers = 0;

    // Calculate Optimal Workers
    for (auto& cc : command_centers)
        optimal_workers += static_cast<int>(std::ceil(1.5f * cc->ideal_harvesters));    // Assume ~50% overproduction for mining
    for (auto& refinery : refineries)
        optimal_workers += refinery->ideal_harvesters;
    optimal_workers = std::min(optimal_workers, maximum_workers);                       // Don't make too many, though

    if (num_workers >= optimal_workers)
        return;

    // Schedule training at our Command Centers
    for (auto& cc : command_centers) {
        if (num_workers++ >= optimal_workers)
            break;

        if (cc->build_progress != 1.0f)
            continue;

        if (!cc->IsIdle())
            continue;

        if (builder_->CountScheduledTrainings(gHub->GetCurrentWorkerType()) > 0)
            continue;

        orders.emplace_back(gAPI->observer().GetUnitTypeData(gHub->GetCurrentWorkerType()), cc);
    }

    if (orders.empty())
        return;

    builder_->ScheduleTrainingOrders(orders, true);
}

void SecureVespeneIncome() {
    Units gas_workers = gAPI->observer().GetUnits(IsWorkerWithJob(Worker::Job::gathering_vespene), sc2::Unit::Alliance::Self);

    // We move max one for each refinery in each call to this function
    for (const auto& expansion : gHub->GetExpansions()) {
        if (expansion->alliance == sc2::Unit::Alliance::Self) {
            for (const auto& refinery : expansion->refineries) {
                if (refinery->assigned_harvesters < refinery->ideal_harvesters) {
                    // NOTE: Home base is updated in Worker::GatherVespene()
                    auto worker = GetClosestFreeWorker(refinery->pos);
                    if (worker) {
                        worker->GatherVespene(refinery);
                    }
                    // Makes sure that we never have more than 3 workers on gas.
                } else if (refinery->assigned_harvesters > refinery->ideal_harvesters) {
                    for (auto it = gas_workers.begin(); it != gas_workers.end(); it++) {
                        if (refinery->tag == (*it)->GetPreviousStepOrders().front().target_unit_tag) {
                            (*it)->AsWorker()->Mine();
                            gas_workers.erase(it);
                            break;
                        }
                    }
                }
            }
        }
    }
}

float SaveEnergy() {
    // If Reasoner wants detection, save as much energy as possible
    auto needed_unitclasses = gReasoner->GetNeededUnitClasses();
    if (std::find(needed_unitclasses.begin(), needed_unitclasses.end(), UnitClass::detection) != needed_unitclasses.end())
        return std::numeric_limits<float>::max();

    // Save one extra scan per 4 minutes of game time (0 scans saved first 4 minutes)
    constexpr int minutes_per_scan_increase = 4;
    float passed_minutes = gAPI->observer().GetGameLoop() / (API::StepsPerSecond * 60.0f);
    return API::OrbitalScanCost * (static_cast<int>(passed_minutes) / minutes_per_scan_increase);
}

void CallDownMULE() {
    auto orbitals = gAPI->observer().GetUnits(
        IsUnit(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND), sc2::Unit::Alliance::Self);
    std::vector<float> usable_energy; // orbitals[i]'s usable energy is in usable_energy[i]

    if (orbitals.empty())
        return;

    // It is important that orbitals are always in the same order, so we end up actually saving
    // (sorting by memory address is okay, as a Unit's memory does not get moved around)
    std::sort(orbitals.begin(), orbitals.end());

    // Calculate the usable energy for each orbital
    usable_energy.resize(orbitals.size());
    for (std::size_t i = 0; i < orbitals.size(); ++i)
        usable_energy[i] = orbitals[i]->energy;

    // Distribute the reserved energy uniformly
    float save_energy = std::min(SaveEnergy(), orbitals.size() * 200.0f);
    while (save_energy > 0.0f) {
        for (std::size_t i = 0; i < orbitals.size() && save_energy > 0.0f; ++i) {
            usable_energy[i] -= API::OrbitalScanCost;
            save_energy -= API::OrbitalScanCost;
        }
    }

    // Call down mules
    auto mineral_patches = gAPI->observer().GetUnits(IsMineralPatch(),
        sc2::Unit::Alliance::Neutral);

    for (std::size_t i = 0; i < orbitals.size(); ++i) {
        if (usable_energy[i] < API::OrbitalMuleCost && orbitals[i]->energy < 200.0f)
            continue;

        auto mineral_target = mineral_patches.GetClosestUnit(orbitals[i]->pos);
        if (!mineral_target)
            continue;

        gAPI->action().Cast(orbitals[i], sc2::ABILITY_ID::EFFECT_CALLDOWNMULE, mineral_target);
    }
}

}  // namespace

void Miner::OnStep(Builder* builder_) {
    // Make all unemployed workers mine
    // Note: Mine also updates the home base of the worker
    Units unemployed_workers = gAPI->observer().GetUnits(IsWorkerWithJob(Worker::Job::unemployed), sc2::Unit::Alliance::Self);
    for (auto& unit : unemployed_workers) {
        unit->AsWorker()->Mine();
    }

    if (gAPI->observer().GetGameLoop() % steps_between_balance == 0)
        BalanceWorkers();
    SecureMineralsIncome(builder_);
    SecureVespeneIncome();

    if (gHub->GetCurrentRace() == sc2::Race::Terran)
        CallDownMULE();
}

void Miner::OnUnitCreated(Unit* unit_) {
    if (IsTownHall()(*unit_)) {
        // Put our TownHall's rally point
        auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
            sc2::Unit::Alliance::Neutral);

        auto mineral_target = units.GetClosestUnit(unit_->pos);
        if (!mineral_target)
            return;

        gAPI->action().Cast(unit_, sc2::ABILITY_ID::RALLY_WORKERS, mineral_target);
    }
}

void Miner::OnUnitDestroyed(Unit* unit_, Builder*) {
    if (IsTownHall()(*unit_)) {
        // A Town Hall died => Reassing workers
        auto expo = gHub->GetClosestExpansion(unit_->pos);
        SplitWorkersOf(expo);
    }

}

void Miner::OnBuildingConstructionComplete(Unit* unit_) {
    // Needed to update SCV:s task after refinery construction has completed
    // (as they start to collect gas directly instead of becoming idle)
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY) {
        auto building_workers = gAPI->observer().GetUnits(IsWorkerWithJob(Worker::Job::building), sc2::Unit::Alliance::Self);
        if (!building_workers.empty()) {
            building_workers.GetClosestUnit(unit_->pos)->AsWorker()->SetAsUnemployed();
        }
    }
}

void Miner::OnUnitIdle(Unit* unit_, Builder*) {
    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::ZERG_DRONE: {
            auto job = unit_->AsWorker()->GetJob();
            if (job == Worker::Job::gathering_minerals ||
                job == Worker::Job::gathering_vespene) {
                unit_->AsWorker()->SetAsUnemployed();
            }
            break;
        }

        case sc2::UNIT_TYPEID::TERRAN_MULE: {
            // Send MULE to closest mineral patch of our Starting Location on idle
            // TODO: Maybe send it to nearest mineral patch of a base belonging to us?
            auto units = gAPI->observer().GetUnits(IsMineralPatch(), sc2::Unit::Alliance::Neutral);
            auto mineral_target = units.GetClosestUnit(gAPI->observer().StartingLocation());
            if (!mineral_target)
                break;

            gAPI->action().Cast(unit_, sc2::ABILITY_ID::SMART, mineral_target);
            break;
        }
        default:
            break;
    }
}

void Miner::SplitWorkersOf(const std::shared_ptr<Expansion>& expansion_) {
    // Assumption: Other bases are already balanced or closed to balanced
    // Then, if we distribute these workers evenly our bases should still be balanced

    // We don't use Hub::GetOurExpansions as we do not want/need them sorted
    Expansions our_active_expansions;
    // Would it be good to reserve e.g. gHub->GetExpansions().size() - 1? If so that should be done in hub too

    for (auto& expo : gHub->GetExpansions()) {
        if (expo->alliance == sc2::Unit::Alliance::Self && expo != expansion_) {
            our_active_expansions.emplace_back(expo);
        }
    }

    if (our_active_expansions.empty())
        return;

    auto workers_at_expansion = gAPI->observer().GetUnits(IsWorkerWithHomeBase(expansion_), sc2::Unit::Alliance::Self);

    // Send workers in round-robin fashion
    auto roundRobinItr = our_active_expansions.begin();

    for (auto& unit : workers_at_expansion) {
        auto worker = unit->AsWorker();

        worker->SetHomeBase(*roundRobinItr);
        if (worker->GetJob() == Worker::Job::gathering_minerals || worker->GetJob() == Worker::Job::gathering_vespene) {
            worker->Mine();
        }

        if (++roundRobinItr == our_active_expansions.end()) {
            roundRobinItr = our_active_expansions.begin();
        }
    }
}

void Miner::BalanceWorkers() {
    Timer timer;
    timer.Start();

    // Build a map of how many mining workers above their ideal each expansion has
    std::multimap<int, std::pair<const std::shared_ptr<Expansion>&, Units>> sorted_expansions; // note: maps are ordered by key

    for (auto& expo : gHub->GetExpansions()) {
        if (expo->alliance == sc2::Unit::Alliance::Self && expo->town_hall->ideal_harvesters != 0) {
            auto mining_workers_at_expansion = gAPI->observer().GetUnits(
                    MultiFilter(MultiFilter::Selector::And, {IsWorkerWithHomeBase(expo), IsWorkerWithJob(Worker::Job::gathering_minerals)}),
                    sc2::Unit::Alliance::Self);
            int over = static_cast<int>(mining_workers_at_expansion.size()) - expo->town_hall->ideal_harvesters;
            auto expansion_worker_pair = std::pair<const std::shared_ptr<Expansion>&, Units>(expo, mining_workers_at_expansion);
            sorted_expansions.emplace(std::make_pair(std::max(0, over), expansion_worker_pair));
        }
    }

    if (sorted_expansions.size() <= 1)
        return;

    // If ideal difference is >= 2 of top and bottom guy => balance workers
    int diff = sorted_expansions.rbegin()->first - sorted_expansions.begin()->first;
    if (diff >= req_imbalance_to_transfer) {
        int move = static_cast<int>(std::ceil(diff / 2.0f));
        int moved = move;
        while (move--) {
            if (auto worker = sorted_expansions.rbegin()->second.second.back()) {
                worker->AsWorker()->SetHomeBase(sorted_expansions.begin()->second.first);
                worker->AsWorker()->Mine();
                sorted_expansions.rbegin()->second.second.pop_back();
            }
        }

        float time = timer.Finish();
        gHistory.debug(LogChannel::economy) << "Moved " << moved << " workers (" << time << " ms spent)" << std::endl;
    }
}
