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
    //all_in,           // Used when are more or less guaranteed to lose if we don't kill our opponent soon
    //scout,            // We are playing blind and scouting/gathering information is a top priority (use scans, send out units around the map etc.)
    greedy              // Used when e.g. we can't break our opponent and he can't do damage to us (or used as a gamble)
};

// Makes assumptions based on reasoning (inferred from memory, etc)
class Reasoner {
    public:
        // Calculates how the bot should play (the play style) based of the gathered information (Memory)
        // and the previous calculated PlayStyle. This should ideally be called once every step
        // (if the function gets to time-consuming it might be worth considering running it in a separate thread)
        PlayStyle CalculatePlayStyle();

        // Returns the latest calculated play style
        PlayStyle GetPlayStyle();

        // Return: Vector of Expansions sorted by likelihood
        std::vector<std::shared_ptr<Expansion>> GetLikelyEnemyExpansions();

    private:
        PlayStyle m_latest_play_style = PlayStyle::normal;
};

extern std::unique_ptr<Reasoner> gReasoner;
