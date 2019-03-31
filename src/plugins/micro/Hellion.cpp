#include "Hellion.h"

#include "core/API.h"

Hellion::Hellion(Unit* unit)
        : DefaultUnit(unit)
{
}

void Hellion::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    const Unit* target = enemies.GetClosestUnit(m_self->pos);

    if (m_self->weapon_cooldown == 0) {
            Cast( sc2::ABILITY_ID::SMART, target);
    } else {
        //TODO Smarter retreat pathing
        MoveTo(sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                 gAPI->observer().StartingLocation().y));
    }

}
