#include "Cyclone.h"

#include "core/API.h"

Cyclone::Cyclone(Unit* unit)
        : MicroPlugin(unit)
{
}

void Cyclone::OnCombatStep(const Units& enemies, const Units& allies) {
    const Unit* closestEnemy = enemies.GetClosestUnit(m_self->pos);
    float closestEnemyDistance = Distance2D(m_self->pos, closestEnemy->pos);

    if(closestEnemyDistance <= LockOnRange) {
        Cast(sc2::ABILITY_ID::EFFECT_LOCKON, closestEnemy);
        //No need to ever move closer than 7, interrupts chasing
        MoveTo(m_self->pos);
    }
    if(closestEnemyDistance > LockOnRange){
        //Chase if enemy is moving away
        MoveTo(closestEnemy->pos);
    }
}
