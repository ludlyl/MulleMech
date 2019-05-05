#include "IntelligenceHolder.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "BuildingPlacer.h"

#include <sc2api/sc2_common.h>

#include <limits>

void IntelligenceHolder::OnUnitEnterVision(Unit* unit_) {
    if (unit_->alliance == sc2::Unit::Alliance::Enemy) {
        if (!IsTemporaryUnit()(*unit_) && !m_enemyUnits.contains(unit_)) {
            m_enemyUnits.push_back(unit_);
            if (IsTownHall()(*unit_)) {
                MarkEnemyExpansion(unit_);
            }
        }
    }
}

void IntelligenceHolder::OnUnitDestroyed(Unit* unit_) {
    // Clear out unit from internal list if dead
    if (unit_->alliance == sc2::Unit::Alliance::Enemy) {
         if (IsTownHall()(*unit_)) {
            for (const auto& i : gHub->GetExpansions()) {
                if (unit_ == i->town_hall) {
                    // We currently don't support making the enemy's main neutral
                    if (i != m_enemy_main_base) {
                        i->alliance = sc2::Unit::Alliance::Neutral;
                    }
                    i->town_hall = nullptr;
                    gHistory.info() << "Enemy lost region: (" << unit_->pos.x << ", " << unit_->pos.y << ")" << std::endl;
                    break;
                }
            }
        }
        m_enemyUnits.remove(unit_);
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

void IntelligenceHolder::MarkEnemyMainBasePosition(const sc2::Point2D& pos_) {
    auto exp = gHub->GetClosestExpansion(pos_);
    exp->alliance = sc2::Unit::Alliance::Enemy;
    m_enemy_main_base = exp;
}

Expansions IntelligenceHolder::GetKnownEnemyExpansions() const {
    if (!gIntelligenceHolder->GetEnemyMainBase()) {
        return {};
    }

    Expansions expos;

    for (auto& expo : gHub->GetExpansions()) {
        if (expo->alliance == sc2::Unit::Alliance::Enemy)
            expos.push_back(expo);
    }

    // Sort bases by how far they are (walkable distance) from the main, with the assumption
    // that such a sorting will tell us which base is the natural, and so forth
    auto starting = gIntelligenceHolder->GetEnemyMainBase();
    std::sort(expos.begin(), expos.end(), [&starting](auto& e1, auto& e2) {
        return starting->distanceTo(e1) < starting->distanceTo(e2);
    });

    return expos;
}

std::shared_ptr<Expansion> IntelligenceHolder::GetLatestKnownEnemyExpansion() const {
    auto expansions = GetKnownEnemyExpansions();
    if (expansions.empty())
        return nullptr;

    return expansions.back();
}

int IntelligenceHolder::GetKnownEnemyExpansionCount() const {
    int count = 0;
    for (auto& expo : gHub->GetExpansions()) {
        if (expo->alliance == sc2::Unit::Alliance::Enemy) {
            count++;
        }
    }
    return count;
}

void IntelligenceHolder::MarkEnemyExpansion(Unit* unit_) {
    assert(IsTownHall()(*unit_));

    // If the main base isn't already set (and the main doesn't get set by calling GetEnemyMainBase()),
    // calculate which starting location is closest to the found expansion (town hall) and set the enemy main to that base
    if (!m_enemy_main_base && !GetEnemyMainBase()) {
        float shortest_distance = std::numeric_limits<float>::max();
        sc2::Point2D main_pos;
        for (const auto& possible_start : gAPI->observer().GameInfo().enemy_start_locations) {
            float distance = sc2::Distance2D(unit_->pos, possible_start);
            if (distance < shortest_distance) {
                shortest_distance = distance;
                main_pos = possible_start;
            }
        }
        m_enemy_main_base = gHub->GetClosestExpansion(main_pos);
        m_enemy_main_base->alliance = sc2::Unit::Alliance::Enemy;
    }

    auto exp = gHub->GetClosestExpansion(unit_->pos);
    if (exp->alliance == sc2::Unit::Alliance::Neutral) {
        exp->alliance = sc2::Unit::Alliance::Enemy;
        exp->town_hall = unit_;
    } else if (exp->alliance == sc2::Unit::Alliance::Enemy) {
        if (exp->town_hall == nullptr) {
            exp->town_hall = unit_;
        // Set the sent in town hall as the expansions town hall if it's closer to the town hall location
        // than the previous town hall. This is needed to support macro town halls
        // DistanceSquared is faster
        } else if (sc2::DistanceSquared2D(unit_->pos, exp->town_hall_location) <
                   sc2::DistanceSquared2D(exp->town_hall->pos, exp->town_hall_location)) {
            exp->town_hall = unit_;
        }
    }
}

const Units& IntelligenceHolder::GetEnemyUnits() const {
    return m_enemyUnits;
}

Units IntelligenceHolder::GetEnemyUnits(unsigned int last_seen_by_game_loop_) const {
    Units units;

    for (auto& unit : units) {
        if (unit->last_seen_game_loop >= last_seen_by_game_loop_) {
            units.push_back(unit);
        }
    }

    return Units();
}

std::unique_ptr<IntelligenceHolder> gIntelligenceHolder;
