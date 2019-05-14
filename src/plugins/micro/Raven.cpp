#include "Raven.h"
#include "core/API.h"
#include "core/Helpers.h"

Raven::Raven(Unit* unit)
    : MicroPlugin(unit) {}

void Raven::OnCombatStep(const Units& enemies, const Units& allies) {
    if (!m_stopped) {
        gAPI->action().Stop(m_self);
        m_stopped = true;
    }

    if (m_armorMissileCooldown > 0)
        m_armorMissileCooldown -= 1.0f / API::StepsPerSecond;
    if (m_turretCooldown > 0)
        m_turretCooldown -= 1.0f / API::StepsPerSecond;

    if (!m_self->GetPreviousStepOrders().empty() && m_self->GetPreviousStepOrders().front().ability_id != sc2::ABILITY_ID::MOVE)
        return;

    // Fly to Center
    auto ally_center = allies.CalculateCircle().first;
    if (sc2::DistanceSquared2D(ally_center, m_self->pos) > OffCenterRange * OffCenterRange) {
        gAPI->action().MoveTo(m_self, ally_center);
        return;
    }

    if (!m_self->GetPreviousStepOrders().empty())
        return;

    // Anti-Armor Missile
    if (m_self->energy >= MissileEnergyCost && m_armorMissileCooldown <= 0) {
        m_armorMissileCooldown = ArmorMissileCooldown; // don't search too often

        // Record highest value target
        int max_value = 0;
        Unit* max_value_target = nullptr;

        for (auto& enemy : enemies) {
            constexpr float maxdist_sq = (ArmorMissileRange + FlyToCastRange) * (ArmorMissileRange + FlyToCastRange);
            if (sc2::DistanceSquared2D(enemy->pos, m_self->pos) > maxdist_sq)
                continue;
            if (std::find(enemy->buffs.begin(), enemy->buffs.end(), AntiArmorMissileDebuffId) != enemy->buffs.end())
                continue;
            if (!IsCombatUnit()(enemy))
                continue;
            // Make sure we wouldn't hit our own units
            if (sc2::DistanceSquared2D(allies.GetClosestUnit(enemy->pos)->pos, enemy->pos) < ArmorMissileRadius * ArmorMissileRadius)
                continue;

            // Value of all affected targets
            int affected_value = 0;
            int count = 0;
            for (auto& inner_enemy : enemies) {
                if (std::find(inner_enemy->buffs.begin(), inner_enemy->buffs.end(), AntiArmorMissileDebuffId) != inner_enemy->buffs.end())
                    continue;
                if (sc2::DistanceSquared2D(enemy->pos, inner_enemy->pos) > ArmorMissileRadius * ArmorMissileRadius)
                    continue;
                if (!IsCombatUnit()(enemy))
                    continue;
                affected_value += inner_enemy->GetValue();
                ++count;
            }

            if (count >= ArmorMissileMinUnits && ArmorMissileMinValue > affected_value && affected_value > max_value) {
                max_value_target = enemy;
                max_value = affected_value;
            }
        }

        if (max_value_target) {
            Cast(AntiArmorMissileId, max_value_target);
            return;
        }
    }

    // Interference Matrix
    if (m_self->energy >= MatrixEnergyCost) {
        Units possible_targets;
        for (auto& enemy : enemies) {
            if (std::find(std::begin(InterferenceMatrixTargets),
                std::end(InterferenceMatrixTargets), enemy->unit_type) == std::end(InterferenceMatrixTargets))
                continue;
            // TODO: Spell does not seem to be in buffs of target...
            /*if (std::find(enemy->buffs.begin(), enemy->buffs.end(), InterferenceMatrixDebuffId) != enemy->buffs.end())
                continue;*/
            constexpr float maxdist_sq = (InterferenceMatrixRange + FlyToCastRange) * (InterferenceMatrixRange + FlyToCastRange);
            if (sc2::DistanceSquared2D(enemy->pos, m_self->pos) > maxdist_sq)
                continue;

            possible_targets.push_back(enemy);
        }
        // Use random to select target (would not be needed if TODO above is fixed)
        if (!possible_targets.empty()) {
            Cast(InterferenceMatrixId, possible_targets.GetRandomUnit());
            return;
        }
    }

    // Auto-turret
    if (m_self->energy >= TurretEnergyCost && m_turretCooldown <= 0) {
        auto enemy = enemies.GetRandomUnit();
        if (!IsCombatUnit()(enemy))
            return;
        float enemy_distance = sc2::Distance2D(enemy->pos, m_self->pos);
        if (enemy_distance > 2 * (BuildTurretRange + FlyToCastRange))
            return; // too far away

        sc2::Point2D direction = enemy->pos - m_self->pos;
        sc2::Normalize2D(direction);
        float distance = std::min(BuildTurretRange + FlyToCastRange, enemy_distance);
        sc2::Point2D target_pos = m_self->pos + direction * distance;

        if (gAPI->query().CanBePlaced(sc2::ABILITY_ID::EFFECT_AUTOTURRET, target_pos)) {
            Cast(sc2::ABILITY_ID::EFFECT_AUTOTURRET, target_pos);
            m_turretCooldown = TurretCooldown;
        }
    }
}

void Raven::OnCombatEnded() {
    m_stopped = false;
    m_armorMissileCooldown = 0;
    m_turretCooldown = 0;
}
