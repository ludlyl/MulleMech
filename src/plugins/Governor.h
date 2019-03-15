// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "../Builder.h"
#include "Plugin.h"

struct Governor : Plugin {
    void OnGameStart(Builder* builder_) final;

    void OnStep(Builder* builder_) final;

    void OnUnitIdle(const sc2::Unit* unit_, Builder* builder_) final;

    void OnBuildingConstructionComplete(const sc2::Unit* unit_) final;

    void Governor::CurrentConsumption();

    private:
        std::list<sc2::UNIT_TYPEID> m_planner_queue;
};
