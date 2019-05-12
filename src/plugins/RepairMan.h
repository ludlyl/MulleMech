// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct RepairMan : Plugin {
    void OnStep(Builder*) final;

    void OnUnitIdle(Unit* unit_, Builder*) final;

    void OnUnitDestroyed(Unit* unit_, Builder* builder_) final;

    // "Upgrades" = upgrades and mutations (i.e. orbital, pf etc.)
    void AddQueuedUpgradesBackIntoBuildingQueue(const Unit* unit_, Builder* builder_) const;

private:
    // How many scv:s that can repair the supplied unit at the same time
    // Note: This function assumes the unit is circular (i.e. it will produce bad results for units like battlecruisers)
    // An scv is needed to get the radius. The scv-radius could be made into a constant instead.
    int GetMaximumScvRepairCountFor(Unit* scv_, Unit* unit_) const;

    // How many scv:s that are ordered to repair the supplied unit
    int CountRepairingScvs(const Units& scvs, Unit* unit_) const;

    static constexpr float BuildingMinHealthCancelRatio = 0.05f; // If health / max health is <= this the building is canceled

    static constexpr int DefaultRepairingScvCount = 3;
};
