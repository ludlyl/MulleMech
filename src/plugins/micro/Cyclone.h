#pragma once

#include "DefaultUnit.h"

class Cyclone : public MicroPlugin {
public:
    explicit Cyclone(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    static constexpr float LockOnRange = 7;
};
