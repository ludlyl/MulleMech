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
        Units copy = enemies;
        auto itr = std::remove_if(copy.begin(), copy.end(), [](const Unit* u) {
            return u->is_flying;
        });
        copy.erase(itr, copy.end());

        if (!copy.empty()) {
            const Unit *target = copy.GetClosestUnit(m_self->pos);
            Attack(target);
    }

    } else {
        //TODO Smarter retreat pathing
        MoveTo(sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                 gAPI->observer().StartingLocation().y));
    }

}
