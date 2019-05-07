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
    // This should only be called when "evacuating" a base or when a base has died
    void SplitWorkersOf(const std::shared_ptr<Expansion>& expansion_);

    // Balance Workers by means of redistribution
    void BalanceWorkers();
};
