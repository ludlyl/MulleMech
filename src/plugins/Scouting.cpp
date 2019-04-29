#include "Scouting.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"
#include "Reasoner.h"
#include "IntelligenceHolder.h"

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
        m_defensiveScv->Mine();
        m_defensiveScv = nullptr;
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
        auto outermostExpansion = gHub->GetOurExpansions().back();
        if (outermostExpansion) {
            m_offensiveScv = GetClosestFreeWorker(outermostExpansion->town_hall_location);
        } else {
            m_offensiveScv = GetClosestFreeWorker(gAPI->observer().StartingLocation());
        }
        if (m_offensiveScv) {
            m_offensiveScv->SetAsScout();

            gAPI->action().Stop(m_offensiveScv); // Why is this needed?

            // Add all potential enemy starting base locations to our scout plan
            m_scoutPhase = ScvScoutPhase::approaching;
            auto locations = gAPI->observer().GameInfo().enemy_start_locations;
            m_unscoutedBases.insert(m_unscoutedBases.end(), locations.begin(), locations.end());
            assert(!m_unscoutedBases.empty() && "Must have at least one enemy start location");

            gHistory.debug(LogChannel::scouting) << "Initiating SCV scouting with " << m_unscoutedBases.size() <<
                                                 " possible enemy base locations" << std::endl;
        }
    }

    if (m_scoutPhase == ScvScoutPhase::not_started)
        return;

    // APPROACHING ENEMY BASE
    if (m_scoutPhase == ScvScoutPhase::approaching && m_offensiveScv->IsIdle()) {
        // If we found main base of enemy; go into exploring mode
        if (gIntelligenceHolder->GetEnemyMainBase()) {
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
                gIntelligenceHolder->MarkEnemyMainBasePosition(m_unscoutedBases.front());
                gHistory.debug(LogChannel::scouting) << "Approaching inferred enemy location" << std::endl;
            } else {
                gHistory.debug(LogChannel::scouting) << "Approaching possible enemy location" << std::endl;
            }

            m_unscoutedBases.erase(m_unscoutedBases.begin());
        }
    }
    // EXPLORING ENEMY BASE
    else if (m_scoutPhase == ScvScoutPhase::explore_enemy_base && m_offensiveScv->IsIdle()) {
        auto mainBase = gIntelligenceHolder->GetEnemyMainBase();

        // Scout the main base of the enemy
        gHistory.debug(LogChannel::scouting) << "Exploring main base" << std::endl;
        ScoutBase(m_offensiveScv, mainBase->town_hall_location);

        // If we haven't seen a natural expansion => go into check for natural state, which will execute
        // after our main base scout finishes
        if (gIntelligenceHolder->GetKnownEnemyExpansionCount() < 2)
            m_scoutPhase = ScvScoutPhase::check_for_natural;
    }
    // CHECKING FOR NATURAL
    else if (m_scoutPhase == ScvScoutPhase::check_for_natural && m_offensiveScv->IsIdle()) {
        auto likelyExpansions = gReasoner->GetLikelyEnemyExpansions();
        assert(!likelyExpansions.empty());
        gAPI->action().MoveTo(m_offensiveScv, likelyExpansions[0]->town_hall_location);
        gHistory.debug(LogChannel::scouting) << "Checking if the natural expansion has been started" << std::endl;

        // Alternate with scouting the main base
        m_scoutPhase = ScvScoutPhase::explore_enemy_base;
    }
}

void Scouting::ScoutBase(Unit* unit, sc2::Point2D base) {
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

    auto scv = GetClosestFreeWorker(gAPI->observer().StartingLocation());
    if (scv) {
        gHistory.debug(LogChannel::scouting) << "Scouting our base for proxy enemy buildings" << std::endl;
        m_defensiveScv = scv;
        m_defensiveScv->SetAsScout();
        ScoutBase(m_defensiveScv, gAPI->observer().StartingLocation());
    }
}

void Scouting::TechScout() {
}

void Scouting::ExpansionScout() {
}
