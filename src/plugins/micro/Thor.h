#pragma once

#include "MicroPlugin.h"

class Thor : public MicroPlugin {
public:
    Thor(Unit* unit);

    void OnCombatStep(const Units& enemies) override;

    static constexpr float MinimumFlyingUnits = 3;
};
