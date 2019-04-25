#include "Raven.h"
#include "core/API.h"

Raven::Raven(Unit* unit)
        : MicroPlugin(unit) {}

void Raven::OnCombatStep(const Units& enemies, const Units& allies) {

    //Use to find missing IDs, will be removed
    auto abilities = gAPI->query().GetAbilitiesForUnit(m_self).abilities;
    auto abilities2 = gAPI->query().GetAbilitiesForUnit(m_self).abilities;
    
    if(m_self->energy>=50){
        Units copy = enemies;
        //Find priority targets (Valuable targets, usually big units)
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
            //TODO Find ID for interference Matrix
            //Cast(sc2::ABILITY_ID::INTERFERENCE_MATRIX, target->pos);




        }

    }




    if(enemies.size()>4 && m_self->energy >= 100){
        const Unit* target = enemies.GetClosestUnit(m_self->pos);

        Cast(sc2::ABILITY_ID::EFFECT_AUTOTURRET, target->pos);
    }

}
