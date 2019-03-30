#include "SiegeTank.h"

#include "core/API.h"

SiegeTank::SiegeTank(Unit* unit)
        : DefaultUnit(unit)
{
}

void SiegeTank::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    // Siege if within siege range and not on top of unit
    if (DistanceSquared2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos) < 14 && DistanceSquared2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos) > 4) {
            Cast(sc2::ABILITY_ID::MORPH_SIEGEMODE);
    }
}
