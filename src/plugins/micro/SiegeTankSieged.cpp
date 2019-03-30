#include "SiegeTankSieged.h"

#include "core/API.h"

SiegeTankSieged::SiegeTankSieged(Unit* unit)
        : DefaultUnit(unit)
{
}

void SiegeTankSieged::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    // Unsiege if out of siege range or if on top of unit
    if ((DistanceSquared2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos) > 14) || (DistanceSquared2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos) < 2)) {
        Cast(sc2::ABILITY_ID::MORPH_UNSIEGE);
    }
}
