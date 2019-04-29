#include "SiegeTank.h"

#include "core/API.h"
SiegeTank::SiegeTank(Unit* unit)
        : MicroPlugin(unit)
{
}

void SiegeTank::OnCombatStep(const Units& enemies, const Units& allies) {

    float closestEnemyDistance = Distance2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos);
    Units ground_enemies = enemies;
    //See if there are any ground units so we don't siege if we can't hit anything
    auto itr = std::remove_if(ground_enemies.begin(), ground_enemies.end(), [](const Unit* u) {
        return u->is_flying;
    });
    ground_enemies.erase(itr, ground_enemies.end());

    if (!ground_enemies.empty()) {
        float closestEnemyDistance = Distance2D(m_self->pos, ground_enemies.GetClosestUnit(m_self->pos)->pos);
        // Siege if within siege range and not on top of unit
        if (closestEnemyDistance <= siegeMaxRange && closestEnemyDistance > siegeMinRange + 1) {
            if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANK) {
                Cast(sc2::ABILITY_ID::MORPH_SIEGEMODE);
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

            if (!enemy_in_sieged_shooting_range) {
                Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
            }
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
