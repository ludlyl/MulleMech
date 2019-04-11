#pragma once

#include "MicroPlugin.h"

class Battlecruiser : public MicroPlugin {
public:
    explicit Battlecruiser(Unit* unit);

    void OnCombatStep(const Units& enemies) override;

    static constexpr float retreatHealth = 275;
};
