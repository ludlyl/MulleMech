//
// Created by kevin on 2019-03-07.
//
#include "Plugin.h"
#include <unordered_set>
#include <vector>

class ReaperHarass : public Plugin {
public:

    ReaperHarass();

    void OnStep(Builder* builder) final;

    void OnUnitIdle(const sc2::Unit* unit, Builder*) final;

    void OnUnitDestroyed(const sc2::Unit* unit, Builder*) final;

    void OnUnitCreated(const sc2::Unit* unit_) final;

    // Scouting records buildings we see into our memory
    void OnUnitEnterVision(const sc2::Unit* unit) final;

private:

    // Early game worker harass
    // Use a group of reaper to scout and attack:
    void WorkerHunt();

    // Attempt to scout around a main base; queues all movement commands at once
    void ScoutBase(sc2::Units units, sc2::Point2D base);

    // DATA

    enum class ReaperStrikePhase {
        not_started,
        approaching,
        explore_enemy_base,
        check_for_natural,
        finished
    };
    ReaperStrikePhase m_ReaperStrikePhase;
    sc2::Units m_reaperStrikeTeam;
    sc2::Tag m_harassReapers;
    std::vector<sc2::Point2D> m_unscoutedBases; // TODO: Should be last scouted timing => reuse for mid game expansion scouting
    std::unordered_set<sc2::Tag> m_seenUnits;   // Units we've seen before
};
