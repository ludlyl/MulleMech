// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct Governor : Plugin {
    void OnGameStart(Builder* builder_) final;

    void OnStep(Builder* builder_) final;

    void OnUnitIdle(Unit* unit_, Builder* builder_) final;

    void OnBuildingConstructionComplete(Unit* unit_) final;

    //Return values are Minerals and Vespene in that order.
    // Return value is in Resource/frame
    std::pair<float, float> CurrentConsumption(Builder* builder_);

    // Counts the total structres of "type" in ALL building queues and on the map
    int CountTotalStructures(Builder* builder_, sc2::UNIT_TYPEID type);

    // Same as above but with units (Units in this case does not include structures aka buildings)
    int CountTotalUnits(Builder* builder_, sc2::UNIT_TYPEID type);
    
private:
    enum class BuildOrderStage {Early, Mid, Finished};

    void AddEarlyGameBuildOrder(Builder* builder_);

    void AddMidGameBuildOrder(Builder* builder_);

    //Used the put all commandcenters in the planner queue at the top of the list
    void PrioritizeCommandCenter();

    std::list<sc2::UNIT_TYPEID> m_planner_queue;
    BuildOrderStage m_build_order_stage = BuildOrderStage::Early;

    static constexpr float ThorsToTanksRatio = 1.f / 3.f;
    static constexpr int OptimalNumOfRavens = 2;
    static constexpr float MedivacsToHellbatsRatio = 1.f / 8.f;
    static constexpr float HellionProductionChance = 0.2f;
    static constexpr int MineralsBufferThreshold = 500;     // If we got this much buffer, move build order stage Early->Mid
    static constexpr int MaxMarines = 8;                    // Never exceed this count of marines
};
