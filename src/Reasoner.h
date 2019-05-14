#pragma once

#include "core/Map.h"

// A (very) general description/assessment of how the bot should play
// TODO: This should be made more advanced/detailed.
//  A start would be to split this enum up into one "EconomicalPlayStyle" and one "MilitaryPlayStyle".
enum class PlayStyle {
    very_defensive,     // E.g. when we have a stronger economy than our opponent and are getting all-ined
    defensive,
    normal,
    offensive,          // We think we have a stronger army than our opponent. This doesn't necessarily mean that we have to throw our army at our opponent
    all_in,             // Used when are more or less guaranteed to lose if we don't kill our opponent soon
    scout,              // We are playing blind and scouting/gathering information is a top priority (use scans, send out units around the map etc.)
    greedy              // Used when e.g. we can't break our opponent and he can't do damage to us (or used as a gamble)
};

enum class UnitClass {
    detection,  // E.g. raven for Terran
    anti_air,   // E.g. thors, vikings, cyclons, mines
    buffer      // Anti-light might be a better name. Basically helions/hellbats for Terran
};

// Makes assumptions based on reasoning (inferred from memory, etc)
class Reasoner {
    public:
        // Calculates how the bot should play (the play style) based of the gathered information (Memory)
        // and the previous calculated PlayStyle. This should ideally be called once every step
        // (if the function gets to time-consuming it might be worth considering running it in a separate thread)
        PlayStyle CalculatePlayStyle();

        // Returns the latest calculated play style
        PlayStyle GetPlayStyle() const;

        const std::vector<UnitClass>& CalculateNeededUnitClasses();

        // Returns what types/classes of units we are currently in need of.
        // Inferred from our knowledge of our opponent (his units & buildings) and from our current units.
        // An empty return vector means that the "normal unit composition" (for the current state of the game) should be built
        // TODO: Return a vector of Pair<UnitClass, Priority> instead (sometimes we might just want a little more
        //  anti-air and sometimes getting as much anti-air as possible might be crucial to survive)
        const std::vector<UnitClass>& GetNeededUnitClasses() const;

        // Return: Vector of Expansions sorted by likelihood (does not contain expansions we know our opponent has)
        std::vector<std::shared_ptr<Expansion>> GetLikelyEnemyExpansions();

    private:
        std::string PlayStyleToString(PlayStyle play_style) const;

        PlayStyle m_latest_play_style = PlayStyle::normal;
        std::vector<UnitClass> m_latest_needed_unit_classes;

        // I.e. we want 50% more unit value that can hit air than our opponent has unit value in air units
        static constexpr float AntiAirToAirUnitValueRatio = 1.5f;

        // How much buffer we want at the very least vs *some* (e.g. doesn't count marine or reaper for now)
        // of our opponents light units
        static constexpr float BufferToEnemyLightUnitValueRatio = 0.5f;

        // If we know our opponent has this many bases/expansions more than us we turn into "extreme measures" (all in or greed)
        static constexpr int ExpansionDisadvantageBeforeExtremeMeasures = 2;

        // If worker count / "ideal workers" < this then we turn into "extreme measures" (all in or greed)
        static constexpr float WorkersToIdealWorkersRatioBeforeExtremeMeasures = 0.25f;

        // If worker count / "known opponent worker count" < this then we turn into "extreme measures" (all in or greed)
        static constexpr float WorkersToOpponentWorkersRatioBeforeExtremeMeasures = 0.3f;

        // How big is the chance that we will greed compared to all in
        static constexpr float GreedToAllInChanceRatio = 0.5f;

        // If our opponent has this much more combat unit value than we do we (are likely to) turn on defensive mode
        // This should be made more advanced though and take things like counters into consideration
        static constexpr float OpponentUnitValueAdvantageBeforeDefensiveRatio = 1.75f;

        // If we have this much more combat unit value than we think our opponent has and have have scouted him recently
        // we (are likely to) turn on offensive mode
        // This should be made more advanced though and take things like counters into consideration
        static constexpr float OurUnitValueAdvantageBeforeOffensiveRatio = 1.75f;

        // We at least need to be above this value for offensive mode to be able to activate
        static constexpr float OffensiveUnitValueThreshold = 2500;

        // We need to have seen "the relevant" town halls of our opponent in the last ... seconds
        static constexpr float OffensiveOpponentTownHallLastSeenLimitInSeconds = 60.f;

        // Our opponent at least need to be above this value for defensive mode to be able to activate
        static constexpr float DefensiveUnitValueThreshold = 500;

        // If we have this many more units than our opponent we (are likely) to turn on scout mode
        static constexpr float ScoutPlayStyleUnitValueRatio = 2.5f;

        // We need to at least be above this value (for our own units) for scout mode to active
        static constexpr float ScoutUnitValueThreshold = 1000;
};

extern std::unique_ptr<Reasoner> gReasoner;
