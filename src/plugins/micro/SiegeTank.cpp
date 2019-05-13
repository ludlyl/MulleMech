#include "SiegeTank.h"

#include "Hub.h"
#include "core/API.h"

SiegeTank::SiegeTank(Unit* unit)
        : MicroPlugin(unit)
{
    m_currentMaxRange = SiegeMaxRange + sc2::GetRandomScalar() * RangeRNG;
    m_unsiegeCooldown = 0;
}

void SiegeTank::OnCombatStep(const Units& enemies, const Units& allies) {
    Units ground_enemies = enemies;
    //See if there are any ground units so we don't siege if we can't hit anything
    auto itr = std::remove_if(ground_enemies.begin(), ground_enemies.end(), [](const Unit* u) {
        return u->is_flying;
    });
    ground_enemies.erase(itr, ground_enemies.end());

    if (!ground_enemies.empty()) {
        auto closest_target = ground_enemies.GetClosestUnit(m_self->pos);
        float closest_target_distance = Distance2D(m_self->pos, closest_target->pos);
        // Siege if within siege range and not on top of unit
        if (closest_target_distance <= m_currentMaxRange && closest_target_distance > SiegeMinRange + 1) {
            if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANK) {
                Cast(sc2::ABILITY_ID::MORPH_SIEGEMODE);
                m_unsiegeCooldown = SiegeLockdownMin + (SiegeLockdownMax - SiegeLockdownMin) * sc2::GetRandomFraction();
                float new_max = SiegeMaxRange + sc2::GetRandomScalar() * RangeRNG;
                m_currentMaxRange = std::max(m_currentMaxRange, new_max);
            } else { // In siege mode
                // Request Scan if we cannot see our target
                auto visibility = gAPI->observer().GetVisibility(closest_target->pos);
                if (closest_target->display_type == sc2::Unit::Snapshot || visibility == sc2::Visibility::Fogged)
                    gHub->RequestScan(closest_target->pos);
            }
        // Unsiege if there aren't any units we can hit while being sieged
        } else if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
            // Unsiege cooldown (does not apply if we're melee attacked)
            if (closest_target_distance > SiegeMinRange && m_unsiegeCooldown > 0) {
                m_unsiegeCooldown -= 1 / API::StepsPerSecond;
                return; // May not unsiege yet
            }

            bool enemy_in_sieged_shooting_range = false;
            float distance_to_enemy;
            for (const auto& enemy : ground_enemies) {
                distance_to_enemy = Distance2D(m_self->pos, enemy->pos);
                if (distance_to_enemy >= SiegeMinRange && distance_to_enemy <= m_currentMaxRange) {
                    enemy_in_sieged_shooting_range = true;
                    break;
                }
            }

            if (!enemy_in_sieged_shooting_range) {
                Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
                m_currentMaxRange -= RangeReduction;
                m_currentMaxRange = std::min(m_currentMaxRange, SiegeMaxRange);
            }
        } else {
            AttackMove();
        }
    } else if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
        Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
    }
}

void SiegeTank::OnCombatEnded() {
    // As the tank might be in the process of sieging,
    // just checking type = TERRAN_SIEGETANKSIEGED before calling this doesn't work
    Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);

    m_currentMaxRange = SiegeMaxRange + sc2::GetRandomScalar() * RangeRNG;
    m_unsiegeCooldown = 0;
}
