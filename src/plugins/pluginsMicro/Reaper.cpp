//
// Created by kevin on 2019-03-14.
//

#include <blueprints/Unit.h>
#include "Reaper.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"

Reaper::Reaper(){
}
void Reaper::OnStep(Builder*){




    auto it2 = std::remove_if(m_reapers.begin(), m_reapers.end(),[](const sc2::Unit* unit_) {


        //Retreat
        if((unit_->health)<(40)){

            gAPI->action().MoveTo(*unit_, sc2::Point2D(gAPI->observer().StartingLocation().x, gAPI->observer().StartingLocation().y));

        }
        else{
            //Bombs
            const sc2::Unit* target = gAPI->observer().GetUnits(sc2::Unit::Alliance::Enemy).GetClosestUnit(unit_->pos);
            if(DistanceSquared2D(target->pos, unit_->pos) < 10){
                gAPI->action().Cast(*unit_, sc2::ABILITY_ID::EFFECT_KD8CHARGE, *target);
            }
        }








    return (unit_->health)<(40);
});


}

void Reaper::OnUnitCreated(const sc2::Unit* unit_) {
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER){

        m_reapers.push_back(unit_);
    }
}

void Reaper::OnUnitDestroyed(const sc2::Unit* unit, Builder*) {
    if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER) {
        auto it = std::remove_if(m_reapers.begin(), m_reapers.end(),[](const sc2::Unit* unit_) {
            return !unit_->is_alive;
        });

        m_reapers.erase(it, m_reapers.end());

    }
}