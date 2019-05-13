#pragma once

#include "MicroPlugin.h"

class Raven : public MicroPlugin {
public:
    explicit Raven(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    void OnCombatEnded() override;

private:
    float m_armorMissileCooldown = 0;
    float m_turretCooldown = 0;
    bool m_stopped = false;

    // Decision Constants
    static constexpr float OffCenterRange = 6.0f;          // Fly back to center of units if we get this far away
    static constexpr float FlyToCastRange = 7.0f;          // How much we're allowed to fly to cast abilities
    static constexpr float ArmorMissileCooldown = 1.5f;    // Cooldown (in secs) for searching for targets (not same as duration which is 21 secs)
    static constexpr float ArmorMissileMinValue = 1200.0f; // Must hit this much value to activate
    static constexpr int ArmorMissileMinUnits = 5;         // Must hit this many units to activate
    static constexpr float TurretCooldown = 4.0f;          // Don't use all energy on turret (cooldown in seconds)

    // Spell Data
    static constexpr float ArmorMissileRange = 10.0f;
    static constexpr float ArmorMissileRadius = 2.88f;
    static constexpr float InterferenceMatrixRange = 9.0f;
    static constexpr float BuildTurretRange = 2.0f;
    static constexpr float TurretEnergyCost = 50.0f;
    static constexpr float MatrixEnergyCost = 50.0f;
    static constexpr float MissileEnergyCost = 75.0f;

    static constexpr sc2::UNIT_TYPEID InterferenceMatrixTargets[] = {
        sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED,
        sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER,
        sc2::UNIT_TYPEID::PROTOSS_CARRIER,
        sc2::UNIT_TYPEID::PROTOSS_IMMORTAL,
        sc2::UNIT_TYPEID::PROTOSS_COLOSSUS,
        sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR,
        sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP,
        sc2::UNIT_TYPEID::PROTOSS_TEMPEST,
        sc2::UNIT_TYPEID::ZERG_INFESTOR
    };

    // IDs missing in API
    static constexpr sc2::ABILITY_ID AntiArmorMissileId      = static_cast<sc2::ABILITY_ID>(3753); // Target: Unit
    static constexpr sc2::ABILITY_ID InterferenceMatrixId    = static_cast<sc2::ABILITY_ID>(3747); // Target: Unit
    static constexpr sc2::BUFF_ID AntiArmorMissileDebuffId   = static_cast<sc2::BUFF_ID>(280);     // The debuff that causes -3 armor
    // static constexpr sc2::BUFF_ID InterferenceMatrixDebuffId = static_cast<sc2::BUFF_ID>();     // TODO: Id unknown
};
