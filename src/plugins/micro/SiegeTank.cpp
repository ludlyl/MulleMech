#include "SiegeTank.h"

#include "core/API.h"

SiegeTank::SiegeTank(Unit* unit)
        : DefaultUnit(unit)
{
}

void SiegeTank::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    int closestEnemyDistance = static_cast<int>(Distance2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos));

    if(m_self->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_SIEGETANK) {
        // Siege if within siege range and not on top of unit
        if (closestEnemyDistance < siegeMaxRange && closestEnemyDistance > siegeMinRange + 1) {
            Cast(sc2::ABILITY_ID::MORPH_SIEGEMODE);
        }

    }

    if(m_self->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED){
        // Unsiege if out of siege range or if on top of unit
        if ((closestEnemyDistance > siegeMaxRange+1) || (closestEnemyDistance < siegeMinRange)) {
            Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
        }
    }
}

void SiegeTank::OnCombatEnded() {
    Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
}
