#pragma once

#include "Unit.h"
#include "Map.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <memory>
#include <optional>
#include <unordered_set>
#include <unordered_map>

// The Planner makes long term commitments, e.g.:
// - reserving resources that have yet to be gathered, or
// - allocating units so no other sub-system uses them
class Planner {
public:
    // Reserve a unit for use; this unit should not be used by other systems
    Unit* ReserveUnit(sc2::UNIT_TYPEID id);

    void ReleaseUnit(const Unit* unit);

    void ReleaseUnit(sc2::Tag tag);

    bool IsUnitReserved(const Unit* unit) const;

    bool IsUnitReserved(sc2::Tag tag) const;

private:
    std::unordered_set<sc2::Tag> m_reservedUnits;
};

// Things we have learned about our opponent
class Memory {
public:
    // Get enemy base location if one has been made; index: 0=>main base, 1=>natural, etc
    std::shared_ptr<Expansion> GetEnemyBase(std::size_t index) const;

    std::shared_ptr<Expansion> GetLatestEnemyBase() const;

    bool EnemyHasBase(std::size_t index) const;

    // Note down where enemy main base is
    void MarkEnemyMainBase(const sc2::Point2D& point);

    // Note down where to find an enemy base
    void MarkEnemyExpansion(const sc2::Point2D& point);

    // Mark the spot of a new enemy building
    void MarkEnemyBuilding(sc2::UNIT_TYPEID type, const sc2::Point3D& pos);

    // Count how many of a certain building does the enemy have (that we've seen)
    int EnemyBuildingCount(sc2::UNIT_TYPEID type);

    // Get buildings of a certain type
    std::vector<sc2::Point3D>& GetEnemyBuildings(sc2::UNIT_TYPEID type);

private:
    std::vector<std::shared_ptr<Expansion>> m_enemyBases;
    std::unordered_map<sc2::UNIT_TYPEID, std::vector<sc2::Point3D>> m_enemyBuildings;
};

class Brain {
public:
    Planner& planner() { return m_planner; }

    Memory& memory() { return m_memory; }

private:
    Planner m_planner;
    Memory m_memory;
};

extern std::unique_ptr<Brain> gBrain;
