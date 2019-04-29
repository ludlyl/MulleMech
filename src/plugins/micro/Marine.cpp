#include "Marine.h"
#include "core/API.h"

Marine::Marine(Unit* unit)
    : DefaultUnit(unit)
{
}

void Marine::OnCombatStep(const Units& enemies, const Units& allies) {
    DefaultUnit::OnCombatStep(enemies, allies);

    // Stim pack
    if (m_self->health / m_self->health_max > StimHpPct) {
        if (!HasBuff(sc2::BUFF_ID::STIMPACK))
            Cast(sc2::ABILITY_ID::EFFECT_STIM);
    }
}
