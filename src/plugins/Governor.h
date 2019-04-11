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

    

    private:
        //Used the put all commandcenters in the planner queue at the top of the list
        void PrioritizeCommandCenter();

        std::list<sc2::UNIT_TYPEID> m_planner_queue;
    
};
