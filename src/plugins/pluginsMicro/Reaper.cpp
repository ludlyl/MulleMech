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



    for(const sc2::Unit* unit_ : m_reapers) {
        sc2::Units enemyUnits = gAPI->observer().GetUnits(sc2::Unit::Alliance::Enemy).operator()();

        //TODO This crashes the program, fix check so reapers avoid attacking buildings
        auto it = std::remove_if(enemyUnits.begin(), enemyUnits.end(),[](const sc2::Unit* unit_) {
            return ((!IsCombatUnit()(*unit_)) && !(IsWorker()(*unit_)));
        });

        enemyUnits.erase(it, enemyUnits.end());
        Units* units = new Units(enemyUnits);


        if(!enemyUnits.empty()){
            const sc2::Unit *target = units->GetClosestUnit(unit_->pos);
            //Retreat
            if ((unit_->health) < (35) && !((gAPI->observer().StartingLocation().x == unit_->pos.x) &&
                                            (gAPI->observer().StartingLocation().y == unit_->pos.y))) {
                if (DistanceSquared2D(target->pos, unit_->pos) < 5) {
                    gAPI->action().Cast(*unit_, sc2::ABILITY_ID::EFFECT_KD8CHARGE, *target);
                }
                gAPI->action().MoveTo(*unit_, sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                           gAPI->observer().StartingLocation().y));

            } else {
                if (unit_->weapon_cooldown == 0) {

                    if (DistanceSquared2D(target->pos, unit_->pos) < 25) {
                        //Bombs
                        gAPI->action().Cast(*unit_, sc2::ABILITY_ID::EFFECT_KD8CHARGE, *target);
                        gAPI->action().Cast(*unit_, sc2::ABILITY_ID::SMART, *target);
                    }
                } else {
                    if (DistanceSquared2D(target->pos, unit_->pos) < 5) {
                        gAPI->action().Cast(*unit_, sc2::ABILITY_ID::EFFECT_KD8CHARGE, *target);
                    }
                    gAPI->action().MoveTo(*unit_, sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                               gAPI->observer().StartingLocation().y));
                }

            }
        }
        else{
            if ((unit_->health) < (35) && !((gAPI->observer().StartingLocation().x == unit_->pos.x) &&
                                            (gAPI->observer().StartingLocation().y == unit_->pos.y))) {
                gAPI->action().MoveTo(*unit_, sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                           gAPI->observer().StartingLocation().y));
            }


        }







    }

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