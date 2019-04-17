#pragma once

#include "MicroPlugin.h"

class SiegeTank : public MicroPlugin {
public:
    explicit SiegeTank(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    void OnCombatEnded() override;

    static constexpr float siegeMinRange = 2;
    static constexpr float siegeMaxRange = 13;
};
