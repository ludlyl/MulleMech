#pragma once

#include "Plugin.h"

#include <unordered_set>
#include <vector>

class Scouting : public Plugin {
public:

    void OnStep(Builder* builder) final;

    void OnUnitIdle(Unit* unit, Builder*) final;

    void OnUnitDestroyed(Unit* unit, Builder*) final;

private:

    // Early game scouting
    // Use an SCV to walk around the enemy's base to try to detect:
    // - Chosen tech
    // - Expand timing
    void EarlyGameScout();

    // Early game scouting
    // Defensive scouting for the early game. E.g.:
    // - Look for hidden proxies close to our base
    // - Look for hidden pylons (cannon rush) in our base vs Protoss
    void ConsiderDefensiveScouting();

    // Send SCV around scouting mid-game
    void MidGameScout();

    // Attempt to scout around a main base; queues all movement commands at once
    void ScoutBase(Unit* unit, sc2::Point2D base);

    // DATA

    enum class ScvScoutPhaseEarlyGame {
        not_started,
        approaching,
        explore_enemy_base,
        check_for_natural,
        finished
    };
    ScvScoutPhaseEarlyGame m_scoutPhaseEarlyGame = ScvScoutPhaseEarlyGame::not_started;
    Worker* m_offensiveScv = nullptr;
    Worker* m_defensiveScv = nullptr;
    std::vector<sc2::Point2D> m_unscoutedBases; // TODO: Should be last scouted timing => reuse for mid game expansion scouting
};
