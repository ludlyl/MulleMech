#include "Scouting.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"
#include "Reasoner.h"

Scouting::Scouting() :
        m_scoutPhase(ScvScoutPhase::not_started), m_offensiveScv(nullptr), m_defensiveScv(nullptr) {
}

void Scouting::OnStep(Builder*) {
    // Early game scouting: if food is >= 15 and we haven't yet found our enemy base
    if (m_scoutPhase != ScvScoutPhase::finished) {
        ScvOffensiveScout();
    }

    ConsiderDefensiveScouting();

    // Use timer based for tech scouting & expansion scouting?
    // TechScout();
    // ExpansionScout();
}

void Scouting::OnUnitIdle(Unit* unit, Builder*) {
    // Return to mining if defensive scout finished
    if (unit == m_defensiveScv) {
        m_defensiveScv = nullptr;
        gBrain->planner().ReleaseUnit(unit);
        // TODO: A hack to trigger OnUnitIdle again, as we can't access the Dispatcher
        gAPI->action().MoveTo(unit, sc2::Point2D(unit->pos.x + 0.5f, unit->pos.y + 0.5f));
    }
}

void Scouting::OnUnitDestroyed(Unit* unit, Builder*) {
    if (unit == m_offensiveScv) {
        gHistory.debug(LogChannel::scouting) << "SCV died, abandoning offensive scouting" << std::endl;
        m_offensiveScv = nullptr;
    }
    if (unit == m_defensiveScv) {
        gHistory.debug(LogChannel::scouting) << "Defensive SCV died" << std::endl;
        m_defensiveScv = nullptr;
    }
}

void Scouting::OnUnitEnterVision(Unit* unit) {
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
                gHistory.info(LogChannel::scouting) << "Found enemy expansion!" << std::endl;
            } else {
                return; // Do not remember the building until we've marked its location
            }
        }
    }

    // Save buildings we've seen, they indicate tech
    if (IsBuilding()(*unit)) {
        if (gBrain->memory().EnemyBuildingCount(unit->unit_type) == 0) {
            gHistory.info(LogChannel::scouting) << "New building type spotted: "
                << sc2::UnitTypeToName(unit->unit_type) << std::endl;
            // TODO: Invoke a callback at the Dispatcher to signify possibly finding new tech?
        }

        gBrain->memory().MarkEnemyBuilding(unit->unit_type, unit->pos);
        m_seenUnits.insert(unit->tag);
    }

    // Save unit types we've seen (but not every unit?), they too indicate tech
    // TODO: ^
}

void Scouting::ScvOffensiveScout() {
    // If our SCV dies during scouting; we consider that finished for now
    // TODO: This seems a bit fragile; maybe we should have code to try again if we never found our enemy,
    // or to determine our enemy's base location by where we were killed and/or the fact he wasn't at the other locations
    if (m_scoutPhase != ScvScoutPhase::not_started && (!m_offensiveScv || !m_offensiveScv->is_alive)) {
        m_scoutPhase = ScvScoutPhase::finished;
        m_offensiveScv = nullptr;
        return;
    }

    // Handle scout phases

    // NOT STARTED
    if (m_scoutPhase == ScvScoutPhase::not_started && gAPI->observer().GetFoodUsed() >= 15) {
        m_offensiveScv = gBrain->planner().ReserveUnit(sc2::UNIT_TYPEID::TERRAN_SCV);
        if (!m_offensiveScv)
            return;

        gAPI->action().Stop(m_offensiveScv);

        // Add all potential enemy base locations to our scout plan
        m_scoutPhase = ScvScoutPhase::approaching;
        auto locations = gAPI->observer().GameInfo().enemy_start_locations;
        m_unscoutedBases.insert(m_unscoutedBases.end(), locations.begin(), locations.end());
        assert(!m_unscoutedBases.empty() && "Must have at least one enemy start location");

        gHistory.debug(LogChannel::scouting) << "Initiating SCV scouting with " << m_unscoutedBases.size() <<
            " possible enemy base locations" << std::endl;
    }

    if (m_scoutPhase == ScvScoutPhase::not_started)
        return;

    // APPROACHING ENEMY BASE
    if (m_scoutPhase == ScvScoutPhase::approaching && m_offensiveScv->orders.empty()) {
        // If we found main base of enemy; go into exploring mode
        if (gBrain->memory().EnemyHasBase(0)) {
            m_scoutPhase = ScvScoutPhase::explore_enemy_base;
            gHistory.debug(LogChannel::scouting) << "Found enemy main base!" << std::endl;
        }
        // Scout next base
        else {
            // Pick closest location (TODO: Maybe instead of air distance, use path distance?)
            std::sort(m_unscoutedBases.begin(), m_unscoutedBases.end(), ClosestToPoint2D(m_offensiveScv->pos));
            gAPI->action().MoveTo(m_offensiveScv, m_unscoutedBases.front());

            // Note down base location if we only have one left
            if (m_unscoutedBases.size() == 1) {
                gBrain->memory().MarkEnemyMainBase(m_unscoutedBases.front());
                gHistory.debug(LogChannel::scouting) << "Approaching inferred enemy location" << std::endl;
            } else {
                gHistory.debug(LogChannel::scouting) << "Approaching possible enemy location" << std::endl;
            }

            m_unscoutedBases.erase(m_unscoutedBases.begin());
        }
    }
    // EXPLORING ENEMY BASE
    else if (m_scoutPhase == ScvScoutPhase::explore_enemy_base && m_offensiveScv->orders.empty()) {
        auto mainBase = gBrain->memory().GetEnemyBase(0);

        // Scout the main base of the enemy
        gHistory.debug(LogChannel::scouting) << "Exploring main base" << std::endl;
        ScoutBase(m_offensiveScv, mainBase->town_hall_location);

        // If we haven't seen a natural expansion => go into check for natural state, which will execute
        // after our main base scout finishes
        if (!gBrain->memory().EnemyHasBase(1))
            m_scoutPhase = ScvScoutPhase::check_for_natural;
    }
    // CHECKING FOR NATURAL
    else if (m_scoutPhase == ScvScoutPhase::check_for_natural && m_offensiveScv->orders.empty()) {
        auto likelyExpansions = gReasoner->GetLikelyEnemyExpansions();
        assert(!likelyExpansions.empty());
        gAPI->action().MoveTo(m_offensiveScv, likelyExpansions[0]->town_hall_location);
        gHistory.debug(LogChannel::scouting) << "Checking if the natural expansion has been started" << std::endl;

        // Alternate with scouting the main base
        m_scoutPhase = ScvScoutPhase::explore_enemy_base;
    }
}

void Scouting::ScoutBase(const Unit* unit, sc2::Point2D base) {
    // TODO(?): This only works if main base is at a different elevation (i.e. has a ramp)
    auto baseHeight = gAPI->observer().TerrainHeight(base);
    auto points = PointsInCircle(18.0f, base, baseHeight);

    bool queue = false;
    for (auto& point : points) {
        gAPI->action().MoveTo(unit, point, queue);
        queue = true;
    }
}

void Scouting::ConsiderDefensiveScouting() {
    // Time to send a new defensive SCV?
    if (gAPI->observer().GetGameLoop() != 1680) // 1:15
        return;

    auto scv = gBrain->planner().ReserveUnit(sc2::UNIT_TYPEID::TERRAN_SCV);
    if (scv) {
        gHistory.debug(LogChannel::scouting) << "Scouting our base for proxy enemy buildings" << std::endl;
        m_defensiveScv = scv;
        ScoutBase(scv, gAPI->observer().StartingLocation());
    }
}

void Scouting::TechScout() {
}

void Scouting::ExpansionScout() {
}
