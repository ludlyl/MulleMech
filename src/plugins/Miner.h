// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "../Builder.h"
#include "Plugin.h"

struct Miner : Plugin {
    void OnStep(Builder* builder_) final;

    void OnUnitCreated(const Unit& unit_) final;

    void OnUnitIdle(const Unit& unit_, Builder*) final;
};
