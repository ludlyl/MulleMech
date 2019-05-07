#include "Raven.h"
#include "core/API.h"

Raven::Raven(Unit* unit)
        : MicroPlugin(unit) {}

void Raven::OnCombatStep(const Units& enemies, const Units& allies) {
    
    if(m_self->energy>=50){
        Units copy = enemies;
        //Find priority targets to disable (Casters and big units)
        auto itr = std::remove_if(copy.begin(), copy.end(), [](const Unit *u) {
            return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_ORACLE ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_CARRIER ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_COLOSSUS||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_VIPER ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_OVERSEER ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_MEDIVAC ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_SIEGETANK ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_THOR ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_THORAP ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER);

        });
        copy.erase(itr, copy.end());

        if(!copy.empty()){
            const Unit* target = enemies.GetClosestUnit(m_self->pos);
            //Needs to be added in sc2client
            //Cast(sc2::ABILITY_ID::EFFECT_INTERFERENCEMATRIX, target->pos);
        }

    }

    Units allyCopy = allies;
    //See if we have units with us with fast attacks, since we play mech this only needs to check for Battlecruisers.
    auto itr = std::remove_if(allyCopy.begin(), allyCopy.end(), [](const Unit *u) {
        return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER);
    });
    allyCopy.erase(itr, allyCopy.end());

    if(!allyCopy.empty()){
        const Unit* target = enemies.GetClosestUnit(m_self->pos);
        //Needs to be added in sc2client
        //Cast(sc2::ABILITY_ID::EFFECT_ANTIARMORMISSILE, target->pos);
    }

    if(enemies.size()>4 && m_self->energy >= 100){
        const Unit* target = enemies.GetClosestUnit(m_self->pos);

        sc2::Point3D targetLocation = target->pos;

        int xChance = rand() % 2 + 1;
        int yChance = rand() % 2 + 1;

        if(xChance == 1){
            targetLocation.x += target->radius;
        }
        else{
            targetLocation.x -= target->radius;
        }

        if(yChance == 1){
            targetLocation.y += target->radius;
        }
        else{
            targetLocation.y -= target->radius;
        }


        Cast(sc2::ABILITY_ID::EFFECT_AUTOTURRET, targetLocation);
    }

}
