// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct QuarterMaster : Plugin {
    void OnStep(Builder* builder_) final;

private:
    float CalcEstimatedDemand(Builder* builder_);
    float CalcEstimatedSupply(Builder* builder_);

    // How many times we will count the currently produced unit(s) to expected supply demand
    static constexpr float ProductionSupplyDemandMultiplier = 2.f;
};
