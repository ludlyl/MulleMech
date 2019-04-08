#pragma once

#include "DefaultUnit.h"

class Cyclone : public MicroPlugin {
public:
    Cyclone(Unit* unit);

    void OnCombatStep(const Units& enemies) override;

    static constexpr float LockOnRange = 7;
};
