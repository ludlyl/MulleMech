#include "Marine.h"

#include "core/API.h"

Marine::Marine(const sc2::Unit* unit)
    : DefaultUnit(unit)
{
}

void Marine::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    // Stim pack
    if (Self()->health / Self()->health_max > 0.7f) {
        if (!HasBuff(sc2::BUFF_ID::STIMPACK))
            Cast(sc2::ABILITY_ID::EFFECT_STIM);
    }
}
