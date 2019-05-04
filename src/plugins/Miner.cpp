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

Unit* GetMovableWorker(const Units& workers) {
    for (auto& worker : workers) {
        if (worker->AsWorker()->GetJob() == Worker::Job::gathering_minerals)
            return worker;
    }
    return nullptr;
}

void SecureMineralsIncome(Builder* builder_) {
    // Cut SCV production on the following playstyles
    switch (gReasoner->GetPlayStyle()) {
        case PlayStyle::all_in:
        case PlayStyle::very_defensive:
            return;
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

// TODO: Workers should use the "home base system" for gas gathering too
//  (currently they'll mine even if the cc at a base gets destroyed)
void SecureVespeneIncome() {
    auto refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self);
    Units gas_workers = gAPI->observer().GetUnits(IsWorkerWithJob(Worker::Job::gathering_vespene), sc2::Unit::Alliance::Self);

    // We move max one for each refinery in each call to this function
    for (const auto& refinery : refineries) {
        if (refinery->assigned_harvesters < refinery->ideal_harvesters) {
            // NOTE: Home base is updated in Worker::GatherVespene()
            auto worker = GetClosestFreeWorker(refinery->pos);
            if (worker) {
                worker->GatherVespene(refinery);
            }
        // Makes sure that we never have more than 3 workers on gas.
        } else if (refinery->assigned_harvesters > refinery->ideal_harvesters) {
            for (auto& gas_worker : gas_workers) {
                if (refinery->tag == gas_worker->GetPreviousStepOrders().front().target_unit_tag) {
                    gas_worker->AsWorker()->Mine();
                    gas_workers.remove(gas_worker);
                    break;
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
    auto mineral_patches = gAPI->observer().GetUnits(IsVisibleMineralPatch(),
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
        // Add TownHall to our map of expansion workers
        auto expo = gHub->GetClosestExpansion(unit_->pos);
        if (m_expansionWorkers.find(expo) == m_expansionWorkers.end())
            m_expansionWorkers.emplace(std::move(expo), Units()); // no workers yet

        // Put our TownHall's rally point
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

        SplitWorkersOf(expo, true);

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
            auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(), sc2::Unit::Alliance::Neutral);
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

void Miner::SplitWorkersOf(const std::shared_ptr<Expansion>& expansion, bool expansionDied) {
    // Assumption: Other bases are already balanced or closed to balanced
    // Then, if we distribute these workers evenly our bases should still be balanced

    auto itr = m_expansionWorkers.find(expansion);
    if (itr == m_expansionWorkers.end())
        return;

    // Only send to other expansions
    std::vector<std::shared_ptr<Expansion>> otherExpansions;
    otherExpansions.reserve(m_expansionWorkers.size());
    for (auto& pair : m_expansionWorkers) {
        // Ignore mined out bases unless we have to reshuffle (i.e. expansion died)
        if (pair.first != expansion && (expansionDied || pair.first->town_hall->ideal_harvesters != 0))
            otherExpansions.push_back(pair.first);
    }

    if (otherExpansions.empty())
        return;

    // Send workers in round-robin fashion
    auto roundRobinItr = otherExpansions.begin();
    for (auto worker_itr = itr->second.begin(); worker_itr != itr->second.end();) {
        auto worker = (*worker_itr)->AsWorker();
        if (!expansionDied &&  worker->GetJob() != Worker::Job::gathering_minerals) {
            ++worker_itr;
            continue;
        }

        worker->SetHomeBase(*roundRobinItr);
        worker->Mine();
        m_expansionWorkers[*roundRobinItr].push_back(worker);
        if (++roundRobinItr == otherExpansions.end())
            roundRobinItr = otherExpansions.begin();

        if (!expansionDied)
            worker_itr = itr->second.erase(worker_itr);
        else
            ++worker_itr;
    }
}

void Miner::BalanceWorkers() {
    if (m_expansionWorkers.size() <= 1)
        return;

    Timer timer;
    timer.Start();

    // Leave expansions with no more mining to do
    AbandonMinedOutBases();

    // Build a map of how many workers above their ideal each expansion has
    std::multimap<int, std::shared_ptr<Expansion>> sortedExpansions; // note: maps are ordered by key

    for (auto& pair : m_expansionWorkers) {
        if (pair.first->town_hall->ideal_harvesters == 0)
            continue; // Skip mined out command centers
        int over = static_cast<int>(pair.second.size()) - IdealWorkerCount(pair.first);
        sortedExpansions.emplace(std::make_pair(std::max(0, over), pair.first));
    }

    if (sortedExpansions.size() <= 1)
        return;

    // If ideal difference is >= 4 of top and bottom guy => balance workers
    int diff = sortedExpansions.rbegin()->first - sortedExpansions.begin()->first;
    if (diff >= req_imbalance_to_transfer) {
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
                std::to_string(IdealWorkerCount(exp.second)) + ")    ";
        }
        gHistory.debug(LogChannel::economy) << str << std::endl;
#endif
    }
}

void Miner::AbandonMinedOutBases() {
    // TODO: Should this work for expired gas geysers too?

    // Abandon empty bases: mine somewhere else
    for (auto& pair : m_expansionWorkers) {
        if (pair.first->town_hall->ideal_harvesters == 0)
            SplitWorkersOf(pair.first, false);
    }
}
