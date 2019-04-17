#include "IntelligenceHolder.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Historican.h"

void IntelligenceHolder::Update() {
    // Add new units
    for (auto& unit : gAPI->observer().GetUnits(Inverse(IsTemporaryUnit{}), sc2::Unit::Alliance::Enemy)) {
        if (!m_enemyUnits.contains(unit)) {
            m_enemyUnits.push_back(unit);
            SaveEnemyBaseLocation(unit);
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

std::shared_ptr<Expansion> IntelligenceHolder::GetEnemyBase(int index) const {
    if (!EnemyHasBase(index))
        return nullptr;

    return m_enemyBases[index];
}

std::shared_ptr<Expansion> IntelligenceHolder::GetLatestEnemyBase() const {
    if (m_enemyBases.empty())
        return nullptr;

    return m_enemyBases[m_enemyBases.size()-1];
}

int IntelligenceHolder::GetEnemyBaseCount() const {
    return static_cast<int>(m_enemyBases.size());
}

bool IntelligenceHolder::EnemyHasBase(int index) const {
    return GetEnemyBaseCount() > index;
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

void IntelligenceHolder::SaveEnemyBaseLocation(Unit* unit) {
    if (IsTownHall()(*unit)) {
        bool main_base = false;
        sc2::Point2D pos = unit->pos;

        // Is it a main base?
        for (auto possible_start : gAPI->observer().GameInfo().enemy_start_locations) {
            if (sc2::Distance2D(unit->pos, possible_start) < 5.0f + unit->radius) {
                main_base = true;
                pos = possible_start;
                break;
            }
        }

        // Is it THE main base or an expansion at another spawn location?
        if (main_base && gIntelligenceHolder->EnemyHasBase(0)) {
            if (sc2::Distance2D(pos, GetEnemyBase(0)->town_hall_location) > 5.0f)
                main_base = false;
        }

        // Save base location
        if (main_base) {
            if (!EnemyHasBase(0))
                MarkEnemyMainBase(pos);
        } else {
            // NOTE: Currently we must know where the main base is before we save expansions
            if (EnemyHasBase(0)) {
                MarkEnemyExpansion(unit->pos);
                gHistory.info(LogChannel::scouting) << "Found enemy expansion!" << std::endl;
            } else {
                return; // Do not remember the building until we've marked its location
            }
        }
    }
}

std::unique_ptr<IntelligenceHolder> gIntelligenceHolder;
