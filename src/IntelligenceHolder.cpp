#include "IntelligenceHolder.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

void IntelligenceHolder::Update() {
    // Add new units
    for (auto& unit : gAPI->observer().GetUnits(Inverse(IsTemporaryUnit{}), sc2::Unit::Alliance::Enemy)) {
        if (!m_enemyUnits.contains(unit)) {
            m_enemyUnits.push_back(unit);
        }
    }

    // Clear out dead units
    auto it = m_enemyUnits.begin();
    while (it != m_enemyUnits.end()) {
        if ((*it)->is_alive) {
            it++;
        } else {
            it = m_enemyUnits.erase(it);
        }
    }
}

std::shared_ptr<Expansion> IntelligenceHolder::GetEnemyBase(std::size_t index) const {
    if (m_enemyBases.size() <= index)
        return nullptr;

    return m_enemyBases[index];
}

std::shared_ptr<Expansion> IntelligenceHolder::GetLatestEnemyBase() const {
    if (m_enemyBases.empty())
        return nullptr;

    return m_enemyBases[m_enemyBases.size()-1];
}

bool IntelligenceHolder::EnemyHasBase(std::size_t index) const {
    return m_enemyBases.size() > index;
}

void IntelligenceHolder::MarkEnemyMainBase(const sc2::Point2D& point) {
    assert(m_enemyBases.empty() && "MarkEnemyMainBase called twice");

    std::shared_ptr<Expansion> exp = gHub->GetClosestExpansion(point);
    exp->alliance = sc2::Unit::Alliance::Enemy;
    m_enemyBases.emplace_back(std::move(exp));
}

void IntelligenceHolder::MarkEnemyExpansion(const sc2::Point2D& point) {
    assert(!m_enemyBases.empty() && "MarkEnemyExpansion called before main base found");

    std::shared_ptr<Expansion> exp = gHub->GetClosestExpansion(point);
    exp->alliance = sc2::Unit::Alliance::Enemy;
    m_enemyBases.emplace_back(std::move(exp));

    // Sort bases by how far they are (walkable distance) from the main, with the assumption
    // that such a sorting will tell us which base is the natural, and so forth
    std::sort(m_enemyBases.begin() + 1, m_enemyBases.end(), [this](auto& a, auto& b) {
        return m_enemyBases[0]->distanceTo(a) < m_enemyBases[0]->distanceTo(b);
    });
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
