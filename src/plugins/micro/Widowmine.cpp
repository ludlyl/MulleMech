#include "Widowmine.h"

#include "core/API.h"
Widowmine::Widowmine(Unit* unit)
        : MicroPlugin(unit)
{
}

void Widowmine::OnCombatStep(const Units& enemies, const Units& allies) {

    float closestEnemyDistance = Distance2D(m_self->pos, enemies.GetClosestUnit(m_self->pos)->pos);

    if(closestEnemyDistance < BurrowRange){
        Cast(sc2::ABILITY_ID::BURROWDOWN_WIDOWMINE);
    }
    else{
        Cast(sc2::ABILITY_ID::BURROWUP_WIDOWMINE);
        }
}

