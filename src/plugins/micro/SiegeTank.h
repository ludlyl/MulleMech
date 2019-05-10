#pragma once

#include "MicroPlugin.h"

class SiegeTank : public MicroPlugin {
public:
    explicit SiegeTank(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    void OnCombatEnded() override;

private:
    float m_currentMaxRange;
    float m_unsiegeCooldown;

    static constexpr float SiegeMinRange = 2;
    static constexpr float SiegeMaxRange = 13;
    static constexpr float RangeRNG = 3.0f; // +/- max range
    static constexpr float RangeReduction = 1.0f; // After unsiege move this much closer
    static constexpr float SiegeLockdownMin = 2.0f;  // Pick from [Min, Max] using RNG to determine how long (in seconds)
    static constexpr float SiegeLockdownMax = 10.0f; // tank must remain sieged after sieging
};
