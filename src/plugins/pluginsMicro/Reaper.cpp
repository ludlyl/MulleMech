//
// Created by kevin on 2019-03-14.
//

#include "Reaper.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"

void Reaper::OnStep(Builder*){


    auto it2 = std::remove_if(m_reapers.begin(), m_reapers.end(),[](const sc2::Unit* unit_) {



        gAPI->action().Cast(unit_, )



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