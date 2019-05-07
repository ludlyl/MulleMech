#include "Widowmine.h"
#include "core/API.h"

Widowmine::Widowmine(Unit* unit)
    : MicroPlugin(unit) {}

void Widowmine::OnCombatStep(const Units& enemies, const Units& allies) {
    float closest_target_dist = sc2::Distance3D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos);

    if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_WIDOWMINE) {
        if (closest_target_dist <= BurrowDownRange)
            Cast(sc2::ABILITY_ID::BURROWDOWN);
        else
            AttackMove();
    } else { // Burrowed
        if (BurrowUpRange < closest_target_dist)
            Cast(sc2::ABILITY_ID::BURROWUP);
    }
}
