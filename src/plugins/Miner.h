// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"
#include "core/Units.h"
#include "core/Map.h"

#include <unordered_map>

struct Miner : Plugin {
    void OnStep(Builder* builder_) final;

    void OnUnitCreated(Unit* unit_) final;

    void OnUnitDestroyed(Unit* unit_, Builder*) final;

    // Needed to update SCV:s task after refinery construction has completed
    // (as they start to collect gas directly instead of becoming idle)
    void OnBuildingConstructionComplete(Unit* unit_) final;

    void OnUnitIdle(Unit* unit_, Builder*) final;

private:
    // Counts both for town halls and refineries
    static int IdealWorkerCount(const std::shared_ptr<Expansion>& expansion);

    static void SecureMineralsIncome(Builder* builder_);

    void SecureVespeneIncome();

    static float SaveEnergy();

    static void CallDownMULE();

    // Set the home base of all workers at the supplied expansion to nullptr if there are no other active expansions
    static void ClearWorkersHomeBaseIfNoActiveExpansion(const std::shared_ptr<Expansion>& expansion_);

    // This should only be called when "evacuating" a base or when a base has died
    static void SplitWorkersOf(const std::shared_ptr<Expansion>& expansion_);

    // Balance Workers by means of redistribution
    static void BalanceWorkers();

    bool m_vespene_gathering_stopped = false;

    static constexpr float MaximumResourceDistance = 10.0f;     // Resources further than this => doesn't belong to this base
    static constexpr int StepsBetweenBalance = 20;              // How often we recalculate SCV balance
    static constexpr int ReqImbalanceToTransfer = 2;            // How many SCVs imbalance we must have before transferring any
    static constexpr int MaximumWorkers = 70;                   // Never go above this number of workers
    static constexpr float VespeneToMineralsStopRatio = 3.f;    // If we have this much more vespene than minerals and have at least VespeneMinimumForStopThreshold vespene we stop gathering gas
    static constexpr float VespeneToMineralsStartRatio = 2.5f;  // Ratio needed for us to start gathering gas again
    static constexpr int VespeneMinimumForStopThreshold = 1200; // If we have less gas than this we don't stop gathering no matter the ratio
    static constexpr int VespeneStartThreshold = 1000;          // If we have less gas than this we start gathering again no matter the ratio
};
