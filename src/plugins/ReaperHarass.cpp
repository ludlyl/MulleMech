#include "ReaperHarass.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"
#include "../BuildingPlacer.h"

ReaperHarass::ReaperHarass() :
    strikeInProgress(false) {
}

void ReaperHarass::OnStep(Builder*) {

    auto it2 = std::remove_if(m_reaperStrikeTeam.begin(), m_reaperStrikeTeam.end(),[](const sc2::Unit* unit_) {
        return (unit_->health)<(40);
    });

    m_reaperStrikeTeam.erase(it2, m_reaperStrikeTeam.end());

    if(m_reaperStrikeTeam.empty()){
        strikeInProgress = false;
    }

    if(strikeInProgress){


    auto it3 = std::remove_if(m_reaperStrikeTeam.begin(), m_reaperStrikeTeam.end(),[](const sc2::Unit* unit_) {

        if(unit_->weapon_cooldown == 0){
            gAPI->action().MoveTo(*unit_, BuildingPlacer::GetCenterBehindMinerals(gBrain->memory().GetLatestEnemyBase()->town_hall_location));
        }
        return false;
    });

    }

    if (m_reaperStrikeTeam.size() > 2 && !strikeInProgress) {
        strikeInProgress = true;
        gHistory.debug(LogChannel::general) << "RAIDERS ROLL" << std::endl;
    }


}

void ReaperHarass::OnUnitCreated(const sc2::Unit* unit_) {
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER && !strikeInProgress){
        gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
                        " added to reaper strike team" << std::endl;

        m_reaperStrikeTeam.push_back(unit_);
    }
}

void ReaperHarass::OnUnitIdle(const sc2::Unit* unit_, Builder*) {
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER && !strikeInProgress){
        gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
                        " added to reaper strike team" << std::endl;
        if ((std::find(m_reaperStrikeTeam.begin(), m_reaperStrikeTeam.end(), unit_) == m_reaperStrikeTeam.end())) {
            m_reaperStrikeTeam.push_back(unit_);
        }
    }
}

void ReaperHarass::OnUnitDestroyed(const sc2::Unit* unit, Builder*) {
    if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER) {
        auto it = std::remove_if(m_reaperStrikeTeam.begin(), m_reaperStrikeTeam.end(),[](const sc2::Unit* unit_) {
            return !unit_->is_alive;
        });

        m_reaperStrikeTeam.erase(it, m_reaperStrikeTeam.end());

        if (m_reaperStrikeTeam.empty()) {
            gHistory.debug(LogChannel::reaperharass) << "Reapers died, mission cancelled" << std::endl;
            strikeInProgress = false;
        }
    }
}