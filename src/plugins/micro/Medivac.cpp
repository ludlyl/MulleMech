#include "Medivac.h"
#include "core/API.h"

Medivac::Medivac(Unit* unit)
    : MicroPlugin(unit) {}

void Medivac::OnCombatStep(const Units& enemies, const Units& allies) {
    // Stop our current movement when we enter combat so we don't fly into the enemy
    if (!m_stopped) {
        gAPI->action().Stop(m_self);
        m_stopped = true;
    }

    Unit* closest_heal_target = nullptr;
    float closest_heal_dist = 0;
    for (auto& ally : allies) {
        if (!ally->HasAttribute(sc2::Attribute::Biological))
            continue;

        if (!closest_heal_target) {
            closest_heal_target = ally;
            closest_heal_dist = sc2::DistanceSquared2D(m_self->pos, ally->pos);
            continue;
        }

        auto other_dist = sc2::DistanceSquared2D(m_self->pos, ally->pos);
        if (other_dist < closest_heal_dist) {
            closest_heal_target = ally;
            closest_heal_dist = other_dist;
        }
    }

    if (!closest_heal_target) {
        // Fly to center of allies (TODO: Should use Squad's already calculated Center)
        gAPI->action().MoveTo(m_self, allies.CalculateCircle().first);
        return;
    }

    if (closest_heal_dist < HealRange * HealRange)
        return;

    Cast(sc2::ABILITY_ID::EFFECT_MEDIVACIGNITEAFTERBURNERS);
    gAPI->action().MoveTo(m_self, closest_heal_target->pos);
}
