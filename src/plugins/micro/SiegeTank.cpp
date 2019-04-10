#include "SiegeTank.h"

#include "core/API.h"

SiegeTank::SiegeTank(Unit* unit)
        : DefaultUnit(unit)
{
}

void SiegeTank::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    float closestEnemyDistance = Distance2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos);
    Units copy = enemies;
    //See if there are any ground units so we don't siege if we can't hit anything
    auto itr = std::remove_if(copy.begin(), copy.end(), [](const Unit* u) {
        return u->is_flying;
    });
    copy.erase(itr, copy.end());

    if (!copy.empty()) {
        // Siege if within siege range and not on top of unit
        if (closestEnemyDistance < siegeMaxRange && closestEnemyDistance > siegeMinRange + 1) {
            Cast(sc2::ABILITY_ID::MORPH_SIEGEMODE);
        }
        else if ((closestEnemyDistance > siegeMaxRange+1) || (closestEnemyDistance < siegeMinRange)) {
            Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
        }
    }
    else{
        Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
    }
}

void SiegeTank::OnCombatEnded() {
    Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
}
