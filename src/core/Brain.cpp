#include "Brain.h"

#include "API.h"
#include "Helpers.h"
#include "Historican.h"
#include "Hub.h"

const sc2::Unit* Planner::ReserveUnit(sc2::UNIT_TYPEID id) {
    auto units = gAPI->observer().GetUnits(IsUnit(id), sc2::Unit::Self);

    for (auto& unit : units()) {
        if (m_reservedUnits.find(unit->tag) != m_reservedUnits.end())
            continue;

        m_reservedUnits.insert(unit->tag);
        return unit;
    }

    return nullptr;
}

void Planner::ReleaseUnit(const sc2::Unit* unit) {
    ReleaseUnit(unit->tag);
}

void Planner::ReleaseUnit(sc2::Tag tag) {
    m_reservedUnits.erase(tag);
}

bool Planner::IsUnitReserved(const sc2::Unit* unit) const {
    return IsUnitReserved(unit->tag);
}

bool Planner::IsUnitReserved(sc2::Tag tag) const {
    return m_reservedUnits.find(tag) != m_reservedUnits.end();
}

std::shared_ptr<Expansion> Memory::GetEnemyBase(std::size_t index) const {
    if (m_enemyBases.size() <= index)
        return nullptr;
    
    return m_enemyBases[index];
}

bool Memory::EnemyHasBase(std::size_t index) const {
    return m_enemyBases.size() > index;
}

void Memory::MarkEnemyMainBase(const sc2::Point2D& point) {
    if (!m_enemyBases.empty()) {
        assert(false && "MarkEnemyMainBase called twice");
        return;
    }
    std::shared_ptr<Expansion> exp = gHub->GetClosestExpansion(point);
    exp->alliance = sc2::Unit::Alliance::Enemy;
    m_enemyBases.emplace_back(std::move(exp));
}

void Memory::MarkEnemyExpansion(const sc2::Point2D& point) {
    if (m_enemyBases.empty()) {
        assert(false && "MarkeEnemyExpansion called before main base found");
        MarkEnemyMainBase(point);
        return;
    }

    std::shared_ptr<Expansion> exp = gHub->GetClosestExpansion(point);
    exp->alliance = sc2::Unit::Alliance::Enemy;
    m_enemyBases.emplace_back(std::move(exp));

    // Sort bases by how far they are (walkabe distance) from the main, with the assumption
    // that such a sorting will tell us which base is the natural, and so forth
    std::sort(m_enemyBases.begin() + 1, m_enemyBases.end(), [this](auto& a, auto& b) {
        return m_enemyBases[0]->distanceTo(a) < m_enemyBases[0]->distanceTo(b);
    });
}

void Memory::MarkEnemyBuilding(sc2::UNIT_TYPEID type, const sc2::Point3D& pos) {
    m_enemyBuildings[type].push_back(pos);
}

int Memory::EnemyBuildingCount(sc2::UNIT_TYPEID type) {
    auto itr = m_enemyBuildings.find(type);
    if (itr == m_enemyBuildings.end())
        return 0;
    return static_cast<int>(itr->second.size());
}

std::vector<sc2::Point3D>& Memory::GetEnemyBuildings(sc2::UNIT_TYPEID type) {
    return m_enemyBuildings[type];
}

std::vector<std::shared_ptr<Expansion>> Reasoning::GetLikelyEnemyExpansions() {
    auto main = gBrain->memory().GetEnemyBase(0);
    if (!main)
        return std::vector<std::shared_ptr<Expansion>>();

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

std::unique_ptr<Brain> gBrain;