#include "Hellion.h"
#include "core/Helpers.h"
#include "core/API.h"

Hellion::Hellion(Unit* unit)
        : DefaultUnit(unit)
{
}

void Hellion::OnCombatStep(const Units& enemies) {
    DefaultUnit::OnCombatStep(enemies);

    if (m_self->weapon_cooldown == 0) {

        //Get rid of flying units
        auto it = std::remove_if(enemies.ToAPI().begin(), enemies.ToAPI().end(), [](const Unit *unit_) {
            return IsFlying()(*unit_);
        });

        enemies.ToAPI().erase(it, enemies.ToAPI().end());

        if (!enemies.ToAPI().empty()) {
        const Unit *target = enemies.GetClosestUnit(m_self->pos);

        Cast(sc2::ABILITY_ID::SMART, target);
    }

    } else {
        //TODO Smarter retreat pathing
        MoveTo(sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                 gAPI->observer().StartingLocation().y));
    }

}
