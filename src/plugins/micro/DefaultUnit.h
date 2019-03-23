#pragma once

#include "MicroPlugin.h"

class DefaultUnit : public MicroPlugin {
public:
    DefaultUnit(Unit* unit);

    void OnCombatStep(const Units& enemies) override;

    static constexpr float FleeHpPct = 0.4f;
};
