#include "Reasoner.h"
#include "Hub.h"
#include "IntelligenceHolder.h"
#include "core/Helpers.h"
#include "core/API.h"
#include "Historican.h"

PlayStyle Reasoner::CalculatePlayStyle() {
    // As a first implementation of this and to keep it simple the logic is base around:
    // If we know our opponent has 2 or more bases than we do, or we have "too few" scvs
    // (compared to our ideal and/or our opponent), then it's a 50% chance that we will
    // all in and a 50% chance that we will greed
    // If we know our opponent has (pretty much) more units than we do we set the play style to "defensive"
    // If we have recently scouted enough of our opponents bases and we think we have more army value we turn on "offensive"
    // When we have much more army value than we've seen of our opponent we turn on scout mode
    // (to not max out on units that are bad vs what our opponent has)

    // TODO: To be able to set the play styles "very_defensive" we need to be able to know if our opponent
    //  does NOT have an expansions at location x and when the last time was that we verified that information

    PlayStyle old_play_style = m_latest_play_style;

    // * Greedy & all in *

    int own_worker_count = static_cast<int>(gAPI->observer().GetUnits(IsWorker(), sc2::Unit::Alliance::Self).size());
    int known_enemy_worker_count = static_cast<int>(gIntelligenceHolder->GetEnemyUnits(IsWorker()).size());

    // I.e. how many workers we need to mine with an "ideal amount" (e.g. 16 on a normal fresh mineral line)
    int minimum_ideal_worker_count = 0;
    auto gather_structures = gAPI->observer().GetUnits(
            MultiFilter(MultiFilter::Selector::Or, {IsTownHall(), IsRefinery()}), sc2::Unit::Alliance::Self);

    for (auto& structure : gather_structures) {
        minimum_ideal_worker_count += structure->ideal_harvesters;
    }

    // Needed as we divide by these variables
    // If we have 0 workers of our own this will make us always go all in or greed to matter what (but is that even bad?)
    known_enemy_worker_count = std::max(1, known_enemy_worker_count);
    minimum_ideal_worker_count = std::max(1, minimum_ideal_worker_count);

    if (gIntelligenceHolder->GetKnownEnemyExpansionCount() >=
        (gHub->GetOurExpansionCount() + ExpansionDisadvantageBeforeExtremeMeasures) ||
        static_cast<float>(own_worker_count) / minimum_ideal_worker_count < WorkersToIdealWorkersRatioBeforeExtremeMeasures ||
        static_cast<float>(own_worker_count) / known_enemy_worker_count < WorkersToOpponentWorkersRatioBeforeExtremeMeasures) {
        if (old_play_style != PlayStyle::greedy && old_play_style != PlayStyle::all_in) {
            if (GreedToAllInChanceRatio > sc2::GetRandomFraction()) {
                m_latest_play_style = PlayStyle::greedy;
            } else {
                m_latest_play_style = PlayStyle::all_in;
            }
        }
    } else {
        float enemy_combat_unit_value = 0;
        float own_combat_unit_value = 0;

        for (auto& unit : gIntelligenceHolder->GetEnemyUnits()) {
            if (IsCombatUnit()(unit))
                enemy_combat_unit_value += unit->GetValue();
        }

        for (auto& unit : gAPI->observer().GetUnits(IsCombatUnit(), sc2::Unit::Alliance::Self)) 
            own_combat_unit_value += unit->GetValue();

        // * Defensive *

        if (enemy_combat_unit_value >= own_combat_unit_value * OpponentUnitValueAdvantageBeforeDefensiveRatio &&
            enemy_combat_unit_value > DefensiveUnitValueThreshold) {
            m_latest_play_style = PlayStyle::defensive;
        }

        // * Offensive *

        // We need to have seen our opponents natural + two latest bases (if they exist) in the specified time
        // TODO: Split this up into functions to make it readable
        else if (own_combat_unit_value > enemy_combat_unit_value * OurUnitValueAdvantageBeforeOffensiveRatio &&
                 own_combat_unit_value > OffensiveUnitValueThreshold) {
            // The town halls should always be set in the expansions we get from this function
            auto opponent_expansions = gIntelligenceHolder->GetKnownEnemyExpansions();
            auto current_game_loop = gAPI->observer().GetGameLoop();
            // Check natural and last two expansions (ok if last two is natural and main)
            if (opponent_expansions.size() >= 2) {
                const auto& natural = opponent_expansions.at(1);
                const auto& second_last = opponent_expansions.at(opponent_expansions.size() - 2);
                const auto& last = opponent_expansions.at(opponent_expansions.size() - 1);
                if ((current_game_loop - natural->town_hall->last_seen_game_loop) /
                    API::StepsPerSecond <= OffensiveOpponentTownHallLastSeenLimitInSeconds &&
                    (current_game_loop - second_last->town_hall->last_seen_game_loop) /
                    API::StepsPerSecond <= OffensiveOpponentTownHallLastSeenLimitInSeconds &&
                    (current_game_loop - last->town_hall->last_seen_game_loop) /
                    API::StepsPerSecond <= OffensiveOpponentTownHallLastSeenLimitInSeconds) {
                    m_latest_play_style = PlayStyle::offensive;
                }
            } else if (!opponent_expansions.empty() &&
                       (current_game_loop - opponent_expansions.front()->town_hall->last_seen_game_loop) /
                       API::StepsPerSecond <= OffensiveOpponentTownHallLastSeenLimitInSeconds) {
                m_latest_play_style = PlayStyle::offensive;
            } else {
                m_latest_play_style = PlayStyle::offensive;
            }
        }

        // * Scout *

        else if (own_combat_unit_value > enemy_combat_unit_value * ScoutPlayStyleUnitValueRatio &&
                 own_combat_unit_value > ScoutUnitValueThreshold) {
            m_latest_play_style = PlayStyle::scout;
        } else {
            m_latest_play_style = PlayStyle::normal;
        }
    }

    if (old_play_style != m_latest_play_style) {
        auto play_style_str = PlayStyleToString(m_latest_play_style);
        gHistory.info(LogChannel::reasoning) << "New play style calculated: " << play_style_str << std::endl;
    }
    return m_latest_play_style;
}

PlayStyle Reasoner::GetPlayStyle() const {
    return m_latest_play_style;
}

const std::vector<UnitClass>& Reasoner::CalculateNeededUnitClasses() {
    // TODO: Add debug messages to when the needed unit classes have changed
    m_latest_needed_unit_classes.clear();

    // * Check detection *
    // The current logic behind if we need more detection is: if we have <= 2 bases,
    // have at least 1 raven we are fine no matter how much invisible units our opponent has.
    // If we have > 2 bases we need at least 1 raven and 1 turret per base to be satisfied
    // An improvement to this logic would be to somehow base the number of turrets/scans/ravens
    // we need on the number of "groups" cloaked units our opponent has
    // (as just basing it on the number would mean e.g. mass burrow ling or mothership would mess things up)

    bool enemy_cloak_potential_exists = false;

    // Should we pass in a "last seen in game loop"? I.e. just base this logic on units seen in the last x minutes?
    // We ignore observers
    for (auto& unit : gIntelligenceHolder->GetEnemyUnits()) {
        if ((unit->cloak == sc2::Unit::CloakState::Cloaked ||
            unit->cloak == sc2::Unit::CloakState::CloakedDetected ||
            unit->unit_type == sc2::UNIT_TYPEID::TERRAN_BANSHEE ||
            unit->unit_type == sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB ||
            unit->unit_type == sc2::UNIT_TYPEID::ZERG_LURKERDENMP ||
            unit->unit_type == sc2::UNIT_TYPEID::ZERG_LURKERMP ||
            unit->unit_type == sc2::UNIT_TYPEID::ZERG_LURKERMPEGG ||
            unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_DARKSHRINE)
            && unit->unit_type != sc2::UNIT_TYPEID::PROTOSS_OBSERVER) {
            enemy_cloak_potential_exists = true;
            break;
        }
    }

    if (enemy_cloak_potential_exists) {
        int number_of_expansions = gHub->GetOurExpansionCount();
        int number_of_ravens = static_cast<int>(gAPI->observer().GetUnits(
                IsUnit(sc2::UNIT_TYPEID::TERRAN_RAVEN), sc2::Unit::Alliance::Self).size());

        // NOTE: The missile turrets requirement is commented out as no code for building them exists yet

        //int number_of_turrets = static_cast<int>(gAPI->observer().GetUnits(
        //        IsUnit(sc2::UNIT_TYPEID::TERRAN_MISSILETURRET), sc2::Unit::Alliance::Self).size());

        if (number_of_expansions <= 2) {
            if (number_of_ravens < 1) {
                m_latest_needed_unit_classes.emplace_back(UnitClass::detection);
            }
        } else if (/*number_of_turrets < number_of_expansions ||*/ number_of_ravens < 1) {
            m_latest_needed_unit_classes.emplace_back(UnitClass::detection);
        }
    }

    // * Check anti-air *
    // For now the basic logic is that we want "AntiAirToAirUnitValueRatio" (50% more when this was first added)
    // more unit value that can hit air units than our opponent has unit value in air units
    // This doesn't take into account turrets at all but they should continuously be built anyway

    float enemy_air_unit_value = 0;
    float own_anti_air_unit_value = 0;

    for (auto& unit : gIntelligenceHolder->GetEnemyUnits()) {
        if (IsCombatUnit()(unit) && unit->is_flying)
            enemy_air_unit_value += unit->GetValue();
    }

    for (auto& unit : gAPI->observer().GetUnits(IsAntiAirUnit(), sc2::Unit::Alliance::Self))
        own_anti_air_unit_value += unit->GetValue();

    if (enemy_air_unit_value * AntiAirToAirUnitValueRatio > own_anti_air_unit_value) {
        m_latest_needed_unit_classes.emplace_back(UnitClass::anti_air);
    }

    // * Check buffer (only added if the needed for anti-air isn't too big) *
    // The logic behind this is really (really) basic right now, just check if our opponent has
    // more than BufferToEnemyLightUnitValueRatio (50% more when this was first added) *light*
    // (we don't count all types) units than we have buffer units
    // If this value is set we should probably prioritize hellbats/helions over mines

    float enemy_light_unit_value = 0;
    float own_buffer_unit_value = 0;

    for (auto& unit : gIntelligenceHolder->GetEnemyUnits()) {
        // Unclear if we want to count hydras and adepts
        switch (unit->unit_type.ToType()) {
            case sc2::UNIT_TYPEID::ZERG_HYDRALISK:
            case sc2::UNIT_TYPEID::ZERG_HYDRALISKBURROWED:
            case sc2::UNIT_TYPEID::ZERG_ZERGLING:
            case sc2::UNIT_TYPEID::ZERG_ZERGLINGBURROWED:
            case sc2::UNIT_TYPEID::PROTOSS_ADEPT:
            case sc2::UNIT_TYPEID::PROTOSS_ZEALOT:
                enemy_light_unit_value += unit->GetValue();
                break;
            default:
                break;
        }
    }

    // A helper function should be used here instead of hard-coding in helions, hellbats etc.
    for (auto& unit : gAPI->observer().GetUnits(IsAntiAirUnit(), sc2::Unit::Alliance::Self)) {
        if (unit->unit_type == sc2::UNIT_TYPEID::TERRAN_HELLION ||
            unit->unit_type == sc2::UNIT_TYPEID::TERRAN_HELLIONTANK ||
            unit->unit_type == sc2::UNIT_TYPEID::TERRAN_MARINE) {
            own_buffer_unit_value += unit->GetValue();
        }
    }

    if (enemy_light_unit_value > own_buffer_unit_value * BufferToEnemyLightUnitValueRatio) {
        m_latest_needed_unit_classes.emplace_back(UnitClass::buffer);
    }

    return m_latest_needed_unit_classes;
}

const std::vector<UnitClass>& Reasoner::GetNeededUnitClasses() const {
    return m_latest_needed_unit_classes;
}

std::vector<std::shared_ptr<Expansion>> Reasoner::GetLikelyEnemyExpansions() {
    auto main = gIntelligenceHolder->GetEnemyMainBase();
    if (!main)
        return {};

    // Assumption: All neutral expansion locations are possible targets
    std::vector<std::shared_ptr<Expansion>> locations;
    locations.reserve(gHub->GetExpansions().size());
    for (auto& expansion : gHub->GetExpansions()) {
        if (expansion->alliance == sc2::Unit::Alliance::Neutral)
            locations.push_back(expansion);
    }

    // Assumption: The closer a base is to the enemy's main base, the more attractive they'll find it
    std::sort(locations.begin(), locations.end(), [&main](auto& a, auto& b) {
        return main->distanceTo(a) < main->distanceTo(b);
    });

    return locations;
}

std::string Reasoner::PlayStyleToString(PlayStyle play_style) const {
    switch (play_style) {
        case PlayStyle::very_defensive:
            return "Very defensive";
        case PlayStyle::defensive:
            return "Defensive";
        case PlayStyle::normal:
            return "Normal";
        case PlayStyle::offensive:
            return "Offensive";
        case PlayStyle::all_in:
            return "All in";
        case PlayStyle::scout:
            return "Scout";
        case PlayStyle::greedy:
            return "Greedy";
    }
}

std::unique_ptr<Reasoner> gReasoner;
