#pragma once

#include "DefaultUnit.h"

class SiegeTank : public DefaultUnit {
public:
    SiegeTank(Unit* unit);

    void OnCombatStep(const Units& enemies) override;

    void OnCombatEnded() override;

    static constexpr float siegeMinRange = 2;
    static constexpr float siegeMaxRange = 13;
};
