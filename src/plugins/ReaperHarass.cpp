#include "ReaperHarass.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"
#include "../BuildingPlacer.h"

ReaperHarass::ReaperHarass() :
        m_ReaperStrikePhase(ReaperStrikePhase::not_started), m_harassReapers(sc2::NullTag) {
}

void ReaperHarass::OnStep(Builder*) {

    auto it2 = std::remove_if(m_reaperStrikeTeam.begin(), m_reaperStrikeTeam.end(),[](const sc2::Unit* unit_) {
        if((unit_->health)<(40)){


            gAPI->action().MoveTo(*unit_, sc2::Point2D(gAPI->observer().StartingLocation().x, gAPI->observer().StartingLocation().y));


        }
        return (unit_->health)<(40);
    });

    m_reaperStrikeTeam.erase(it2, m_reaperStrikeTeam.end());
    if(m_reaperStrikeTeam.empty()){
        m_ReaperStrikePhase = ReaperStrikePhase::not_started;
    }

    if (m_ReaperStrikePhase != ReaperStrikePhase::finished && m_reaperStrikeTeam.size() > 4) {
        WorkerHunt();
        gHistory.debug(LogChannel::general) << "RAIDERS ROLL" << std::endl;
    }
}

void ReaperHarass::OnUnitCreated(const sc2::Unit* unit_) {
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER && m_ReaperStrikePhase == ReaperStrikePhase::not_started){
        gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
                        " added to reaper strike team" << std::endl;

        m_reaperStrikeTeam.push_back(unit_);
    }
}

void ReaperHarass::OnUnitIdle(const sc2::Unit* unit_, Builder*) {
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER && m_ReaperStrikePhase == ReaperStrikePhase::not_started){
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
            m_harassReapers = sc2::NullTag;
            m_ReaperStrikePhase = ReaperStrikePhase::not_started;
        }
    }
}

void ReaperHarass::WorkerHunt() {
    // Clean up dead bodies.
    auto it = std::remove_if(m_reaperStrikeTeam.begin(), m_reaperStrikeTeam.end(),[](const sc2::Unit* unit_) {
        return !unit_->is_alive;
    });

    m_reaperStrikeTeam.erase(it, m_reaperStrikeTeam.end());

    auto reaper = m_reaperStrikeTeam.front();

    // If our Reapers die, mark phase as ready to begin again
    if (m_ReaperStrikePhase != ReaperStrikePhase::not_started && m_reaperStrikeTeam.empty()) {
        m_ReaperStrikePhase = ReaperStrikePhase::not_started;
        m_harassReapers = sc2::NullTag;
        return;
    }

    // Handle scout phases

    // NOT STARTED
    if (m_ReaperStrikePhase == ReaperStrikePhase::not_started) {
        reaper = m_reaperStrikeTeam.front();
        if (!reaper)
            return;
        m_harassReapers = m_reaperStrikeTeam.front()->tag;

        gAPI->action().Stop(m_reaperStrikeTeam);

        // Add all potential enemy base locations to our scout plan
        m_ReaperStrikePhase = ReaperStrikePhase::approaching;
        auto locations = gAPI->observer().GameInfo().enemy_start_locations;
        m_unscoutedBases.insert(m_unscoutedBases.end(), locations.begin(), locations.end());
        assert(!m_unscoutedBases.empty() && "Must have at least one enemy start location");

        gHistory.debug(LogChannel::reaperharass) << "Initiating Reaper scouting with " << m_unscoutedBases.size() <<
                                             " possible enemy base locations" << std::endl;
    }
        // APPROACHING ENEMY BASE
    else if (m_ReaperStrikePhase == ReaperStrikePhase::approaching && m_reaperStrikeTeam.front()->orders.empty()) {
        // If we found main base of enemy; go into exploring mode
        if (gBrain->memory().EnemyHasBase(0)) {
            m_ReaperStrikePhase = ReaperStrikePhase::explore_enemy_base;
            gHistory.debug(LogChannel::reaperharass) << "Reaper found enemy main base!" << std::endl;
        }
            // Scout next base
        else {
            // Pick closest location (TODO: Maybe instead of air distance, use path distance?)
            std::sort(m_unscoutedBases.begin(), m_unscoutedBases.end(), ClosestToPoint2D(m_reaperStrikeTeam.front()->pos));
            gAPI->action().MoveTo(m_reaperStrikeTeam, m_unscoutedBases.front());

            // Note down base location if we only have one left
            if (m_unscoutedBases.size() == 1) {
                gBrain->memory().MarkEnemyMainBase(m_unscoutedBases.front());
                gHistory.debug(LogChannel::reaperharass) << "Reapers approaching inferred enemy location" << std::endl;
            } else {
                gHistory.debug(LogChannel::reaperharass) << "Reapers approaching possible enemy location" << std::endl;
            }

            m_unscoutedBases.erase(m_unscoutedBases.begin());
        }
    }
        // EXPLORING ENEMY BASE
    else if (m_ReaperStrikePhase == ReaperStrikePhase::explore_enemy_base && m_reaperStrikeTeam.front()->orders.empty()) {

        gAPI->action().Attack(m_reaperStrikeTeam, BuildingPlacer::GetCenterBehindMinerals(gBrain->memory().GetLatestEnemyBase()->town_hall_location));

    }
        // CHECKING FOR NATURAL
    else if (m_ReaperStrikePhase == ReaperStrikePhase::check_for_natural && m_reaperStrikeTeam.front()->orders.empty()) {
        auto likelyExpansions = gBrain->reasoning().GetLikelyEnemyExpansions();
        assert(!likelyExpansions.empty());
        gAPI->action().MoveTo(m_reaperStrikeTeam, likelyExpansions[0]->town_hall_location);
        gHistory.debug(LogChannel::reaperharass) << "Reapers checking if the natural expansion has been started" << std::endl;

        // Alternate with scouting the main base
        m_ReaperStrikePhase = ReaperStrikePhase::explore_enemy_base;
    }
}