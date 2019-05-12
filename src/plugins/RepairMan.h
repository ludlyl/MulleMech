// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct RepairMan : Plugin {
    void OnStep(Builder* builder_) final;

    void OnUnitDestroyed(Unit* unit_, Builder* builder_) final;

    // "Upgrades" = upgrades and mutations (i.e. orbital, pf etc.)
    void AddQueuedUpgradesBackIntoBuildingQueue(const Unit* unit_, Builder* builder_) const;

private:
    static constexpr float BuildingMinHealthCancelRatio = 0.05f; // If health / max health is <= this the building is canceled
};
