#pragma once

#include "MicroPlugin.h"

class Thor : public MicroPlugin {
public:
    explicit Thor(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    static constexpr float MinimumFlyingUnits = 3;
};
