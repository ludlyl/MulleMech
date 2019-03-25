// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "../Builder.h"
#include "Plugin.h"

struct Governor : Plugin {
    void OnGameStart(Builder* builder_) final;

    void OnStep(Builder* builder_) final;

    void OnUnitIdle(Unit* unit_, Builder* builder_) final;

    void OnBuildingConstructionComplete(Unit* unit_) final;
};
