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

class Brain {
public:
    Planner& planner() { return m_planner; }

private:
    Planner m_planner;
};

extern std::unique_ptr<Brain> gBrain;
