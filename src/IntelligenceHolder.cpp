#include "IntelligenceHolder.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Historican.h"

#include <sc2api/sc2_common.h>

#include <limits>

void IntelligenceHolder::Update() {
    // Add new units
    for (auto& unit : gAPI->observer().GetUnits(Inverse(IsTemporaryUnit{}), sc2::Unit::Alliance::Enemy)) {
        if (!m_enemyUnits.contains(unit)) {
            m_enemyUnits.push_back(unit);
            if (IsTownHall()(*unit)) {
                MarkEnemyExpansion(unit);
            }
        }
    }

    // Clear out dead units
    // TODO: This seem to be bugged for helions
    auto it = m_enemyUnits.begin();
    while (it != m_enemyUnits.end()) {
        if ((*it)->is_alive) {
            it++;
        } else {
            it = m_enemyUnits.erase(it);
        }
    }
}

std::shared_ptr<Expansion> IntelligenceHolder::GetEnemyMainBase() {
    // If the main base isn't already set and there is only one enemy starting location, set the main base to that location
    if (!m_enemy_main_base && gAPI->observer().GameInfo().enemy_start_locations.size() == 1) {
        m_enemy_main_base = gHub->GetClosestExpansion(gAPI->observer().GameInfo().enemy_start_locations.front());
        m_enemy_main_base->alliance = sc2::Unit::Alliance::Enemy;
    }

    return m_enemy_main_base;
}

std::shared_ptr<Expansion> IntelligenceHolder::GetLatestKnownEnemyExpansion() const {
    auto expansions = gHub->GetKnownEnemyExpansions();
    if (expansions.empty())
        return nullptr;

    return expansions.back();
}

int IntelligenceHolder::GetEnemyExpansionCount() const {
    return static_cast<int>(gHub->GetKnownEnemyExpansions().size());
}

void IntelligenceHolder::MarkEnemyExpansion(const sc2::Point2D& pos) {
    // If the main base isn't already set (and the main doesn't get set by calling GetEnemyMainBase()),
    // calculate which starting location is closest to the found expansion (town hall) and set the enemy main to that base
    if (!m_enemy_main_base && !GetEnemyMainBase()) {
        float shortest_distance = std::numeric_limits<float>::max();
        sc2::Point2D main_pos;
        for (const auto& possible_start : gAPI->observer().GameInfo().enemy_start_locations) {
            float distance = sc2::Distance2D(pos, possible_start);
            if (distance < shortest_distance) {
                shortest_distance = distance;
                main_pos = possible_start;
            }
        }
        m_enemy_main_base = gHub->GetClosestExpansion(main_pos);
        m_enemy_main_base->alliance = sc2::Unit::Alliance::Enemy;
    }

    std::shared_ptr<Expansion> exp = gHub->GetClosestExpansion(pos);
    exp->alliance = sc2::Unit::Alliance::Enemy;
}

void IntelligenceHolder::MarkEnemyExpansion(Unit* unit) {
    MarkEnemyExpansion(unit->pos);
}

const Units& IntelligenceHolder::GetEnemyUnits() const {
    return m_enemyUnits;
}

Units IntelligenceHolder::GetEnemyUnits(unsigned int lastSeenByGameLoop) const {
    Units units;

    for (auto& unit : units) {
        if (unit->last_seen_game_loop >= lastSeenByGameLoop) {
            units.push_back(unit);
        }
    }

    return Units();
}

std::unique_ptr<IntelligenceHolder> gIntelligenceHolder;
