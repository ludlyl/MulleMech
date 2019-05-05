#include "SiegeTank.h"

#include "Hub.h"
#include "core/API.h"

SiegeTank::SiegeTank(Unit* unit)
        : MicroPlugin(unit)
{
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
        if (closest_target_distance <= siegeMaxRange && closest_target_distance > siegeMinRange + 1) {
            if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANK) {
                Cast(sc2::ABILITY_ID::MORPH_SIEGEMODE);
            } else { // In siege mode
                // Request Scan if we cannot see our target
                if (closest_target->display_type == sc2::Unit::Snapshot || !closest_target->IsInVision)
                    gHub->RequestScan(closest_target->pos);
            }
        // Unsiege if there aren't any units we can hit while being sieged
        } else if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
            bool enemy_in_sieged_shooting_range = false;
            float distance_to_enemy;
            for (const auto& enemy : ground_enemies) {
                distance_to_enemy = Distance2D(m_self->pos, enemy->pos);
                if (distance_to_enemy >= siegeMinRange && distance_to_enemy <= siegeMaxRange) {
                    enemy_in_sieged_shooting_range = true;
                    break;
                }
            }

            if (!enemy_in_sieged_shooting_range)
                Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
        } else {
            AttackMove();
        }
    } else if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
        Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
    }
}

void SiegeTank::OnCombatEnded() {
    if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
        Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
    }
}
