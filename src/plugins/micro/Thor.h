#pragma once

#include "MicroPlugin.h"

class Thor : public MicroPlugin {
public:
    explicit Thor(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

private:
    // Returns true if the mood was switched
    bool SwitchAntiAirWeaponIfNeeded(const Units& enemies);

    // How many light flying units that have to be in proximity for explosive anti air mode to be activated
    // (there are more requirements for modes to be switched too)
    static constexpr float ExplosiveModeActivationThreshold = 10;

    static constexpr float ExplosiveModeDeactivationThreshold = 5;

    // As we can only get the range of the currently activated anti air weapon
    // from the type data we have to hard-code these values
    // (We can always get the ground attacks range but it would be a bit inconsistent to not hard-code it as well)
    // Squaring these and comparing with DistanceSquared would be a bit faster
    static constexpr float ExplosivePayloadRange = 10;
    static constexpr float HighImpactPayloadRange = 11;
    static constexpr float GroundAttackRange = 7;

    static constexpr float LightFlyingProximityRange = ExplosivePayloadRange * 1.5f;
};
