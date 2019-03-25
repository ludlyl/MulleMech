#include "Marine.h"

#include "core/API.h"

Marine::Marine(const Unit& unit)
    : DefaultUnit(unit)
{
}

void Marine::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    // Stim pack
    if (m_self->health / m_self->health_max > StimHpPct) {
        if (!HasBuff(sc2::BUFF_ID::STIMPACK))
            Cast(sc2::ABILITY_ID::EFFECT_STIM);
    }
}
