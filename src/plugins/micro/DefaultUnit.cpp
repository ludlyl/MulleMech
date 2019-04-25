#include "DefaultUnit.h"
#include "core/API.h"

DefaultUnit::DefaultUnit(Unit* unit)
    : MicroPlugin(unit) {}

void DefaultUnit::OnCombatStep(const Units& enemies, const Units& allies) {
    if (m_self->health / m_self->health_max > FleeHpPct) {
        if (auto target = enemies.GetClosestUnit(m_self->tag)) {
            Attack(target);
        }
    } else {
        // TODO: Add a function to Units to calculate retreat vector, that is a direction away from the enemies
        //       towards safety and use that instead
        if (!IsMoving())
            MoveTo(gAPI->observer().StartingLocation());
    }
}
