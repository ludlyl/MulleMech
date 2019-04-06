#pragma once

#include "DefaultUnit.h"

class Thor : public DefaultUnit {
public:
    Thor(Unit* unit);

    void OnCombatStep(const Units& enemies) override;

    static constexpr float minimumFlyingUnits = 3;
};
