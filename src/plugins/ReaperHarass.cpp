#include "ReaperHarass.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"

ReaperHarass::ReaperHarass() :
        m_ReaperStrikePhase(ReaperStrikePhase::not_started), m_harassReapers(sc2::NullTag) {
}

void ReaperHarass::OnStep(Builder*) {

    if (m_ReaperStrikePhase != ReaperStrikePhase::finished && m_reaperStrikeTeam.size() > 4) {
        WorkerHunt();
        gHistory.debug(LogChannel::reaperharass) << "RAIDERS ROLL" << std::endl;
    }
}

void ReaperHarass::OnUnitCreated(const sc2::Unit* unit_) {
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER && m_ReaperStrikePhase == ReaperStrikePhase::not_started){
        gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
                        " added to reaper strike team" << std::endl;

        m_reaperStrikeTeam.push_back(unit_);
    }
}

void ReaperHarass::OnUnitIdle(const sc2::Unit* unit, Builder*) {
//
}

void ReaperHarass::OnUnitDestroyed(const sc2::Unit* unit, Builder*) {
    if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER) {
        if (m_reaperStrikeTeam.empty()) {
            gHistory.debug(LogChannel::reaperharass) << "Reapers died, mission cancelled" << std::endl;
            m_harassReapers = sc2::NullTag;
        }
    }
}

void ReaperHarass::OnUnitEnterVision(const sc2::Unit* unit) {
    // Ignore units we've seen before
    if (m_seenUnits.find(unit->tag) != m_seenUnits.end())
        return;

    // Save base locations of our enemy
    if (IsTownHall()(*unit)) {
        bool main_base = false;
        sc2::Point2D pos = unit->pos;

        // Is it a main base?
        for (auto possible_start : gAPI->observer().GameInfo().enemy_start_locations) {
            if (sc2::Distance2D(unit->pos, possible_start) < 5.0f + unit->radius) {
                main_base = true;
                pos = possible_start;
                break;
            }
        }

        // Is it THE main base or an expansion at another spawn location?
        if (main_base && gBrain->memory().EnemyHasBase(0)) {
            if (sc2::Distance2D(pos, gBrain->memory().GetEnemyBase(0)->town_hall_location) > 5.0f)
                main_base = false;
        }

        // Save base location
        if (main_base) {
            if (!gBrain->memory().EnemyHasBase(0))
                gBrain->memory().MarkEnemyMainBase(pos);
        } else {
            // NOTE: Currently we must know where the main base is before we save expansions
            if (gBrain->memory().EnemyHasBase(0)) {
                gBrain->memory().MarkEnemyExpansion(unit->pos);
                gHistory.info(LogChannel::reaperharass) << "Found enemy expansion!" << std::endl;
            } else {
                return; // Do not remember the building until we've marked its location
            }
        }
    }

    // Save buildings we've seen, they indicate tech
    if (IsBuilding()(*unit)) {
        if (gBrain->memory().EnemyBuildingCount(unit->unit_type) == 0) {
            gHistory.info(LogChannel::reaperharass) << "New building type spotted: "
                                                << sc2::UnitTypeToName(unit->unit_type) << std::endl;
            // TODO: Invoke a callback at the Dispatcher to signify possibly finding new tech?
        }

        gBrain->memory().MarkEnemyBuilding(unit->unit_type, unit->pos);
        m_seenUnits.insert(unit->tag);
    }

    // Save unit types we've seen (but not every unit?), they too indicate tech
    // TODO: ^
}

void ReaperHarass::WorkerHunt() {
    // Clean up dead bodies.
    auto it = std::remove_if(m_reaperStrikeTeam.begin(), m_reaperStrikeTeam.end(),[](const sc2::Unit* unit_) {
        return !unit_->is_alive;
    });

    auto reaper = m_reaperStrikeTeam.front();

    // If our SCV dies during scouting; we consider that finished for now
    // TODO: This seems a bit fragile; maybe we should have code to try again if we never found our enemy,
    // or to determine our enemy's base location by where we were killed and/or the fact he wasn't at the other locations
    if (m_ReaperStrikePhase != ReaperStrikePhase::not_started && m_reaperStrikeTeam.empty()) {
        m_ReaperStrikePhase = ReaperStrikePhase::finished;
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
        auto mainBase = gBrain->memory().GetEnemyBase(0);

        // Scout the main base of the enemy
        gHistory.debug(LogChannel::reaperharass) << "Reapers exploring main base" << std::endl;
        ScoutBase(m_reaperStrikeTeam, mainBase->town_hall_location);

        // If we haven't seen a natural expansion => go into check for natural state, which will execute
        // after our main base scout finishes
        if (!gBrain->memory().EnemyHasBase(1))
            m_ReaperStrikePhase = ReaperStrikePhase::check_for_natural;
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

void ReaperHarass::ScoutBase(sc2::Units units, sc2::Point2D base) {
    // TODO(?): This only works if main base is at a different elevation (i.e. has a ramp)
    auto baseHeight = gAPI->observer().TerrainHeight(base);
    auto points = PointsInCircle(18.0f, base, baseHeight);

    bool queue = false;
    for (auto& point : points) {
        gAPI->action().MoveTo(units, point, queue);
        queue = true;
    }
}
