#pragma once

#include "Plugin.h"
#include <unordered_set>
#include <vector>

class Scouting : public Plugin {
public:

    Scouting();

    void OnStep(Builder* builder) final;

    void OnUnitIdle(const Unit& unit, Builder*) final;

    void OnUnitDestroyed(const Unit& unit, Builder*) final;

    // Scouting records buildings we see into our memory
    void OnUnitEnterVision(const Unit& unit) final;

private:

    // Early game scouting
    // Use an SCV to walk around the enemy's base to try to detect:
    // - Chosen tech
    // - Expand timing
    void ScvOffensiveScout();

    // Early game scouting
    // Defensive scouting for the early game. E.g.:
    // - Look for hidden proxies close to our base
    // - Look for hidden pylons (cannon rush) in our base vs Protoss
    void ConsiderDefensiveScouting();

    // Mid and Late game Scouting
    // Use Scans over known base locations with priority of main base & natural to try to
    // determine tech opponent has. Try to scan locations that were least recently scanned.
    void TechScout();

    // Mid and Late game Scouting
    // Send inexpensive units (e.g. marine) to locations we think the enemy might expand to
    // but we have not yet seen him do so.
    void ExpansionScout();

    // Attempt to scout around a main base; queues all movement commands at once
    void ScoutBase(const Unit& unit, sc2::Point2D base);

    // DATA

    enum class ScvScoutPhase {
        not_started,
        approaching,
        explore_enemy_base,
        check_for_natural,
        finished
    };
    ScvScoutPhase m_scoutPhase;
    sc2::Tag m_offensiveScv;
    sc2::Tag m_defensiveScv;
    std::vector<sc2::Point2D> m_unscoutedBases; // TODO: Should be last scouted timing => reuse for mid game expansion scouting
    std::unordered_set<sc2::Tag> m_seenUnits;   // Units we've seen before
};
