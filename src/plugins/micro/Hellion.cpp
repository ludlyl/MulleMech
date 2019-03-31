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


        sc2::Units reachableEnemiesAPI = enemies.ToAPI();
        //Get ground units
        auto it = std::remove_if(reachableEnemiesAPI.begin(), reachableEnemiesAPI.end(), [](const Unit *unit_) {
            return unit_->is_flying;
        });


        reachableEnemiesAPI.erase(it, reachableEnemiesAPI.end());

        Units reachableEnemies(reachableEnemiesAPI);

        if (!reachableEnemies.empty()) {
        const Unit *target = reachableEnemies.GetClosestUnit(m_self->pos);

        Cast(sc2::ABILITY_ID::SMART, target);
    }

    } else {
        //TODO Smarter retreat pathing
        MoveTo(sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                 gAPI->observer().StartingLocation().y));
    }

}
