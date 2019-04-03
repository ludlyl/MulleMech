#include "Brain.h"
#include "API.h"
#include "Helpers.h"
#include "Historican.h"
#include "Hub.h"

Unit* Planner::ReserveUnit(sc2::UNIT_TYPEID id) {
    auto units = gAPI->observer().GetUnits(IsUnit(id), sc2::Unit::Self);

    for (auto& unit : units) {
        if (m_reservedUnits.find(unit->tag) != m_reservedUnits.end())
            continue;

        m_reservedUnits.insert(unit->tag);
        return unit;
    }

    return nullptr;
}

void Planner::ReleaseUnit(const Unit* unit) {
    ReleaseUnit(unit->tag);
}

void Planner::ReleaseUnit(sc2::Tag tag) {
    m_reservedUnits.erase(tag);
}

bool Planner::IsUnitReserved(const Unit* unit) const {
    return IsUnitReserved(unit->tag);
}

bool Planner::IsUnitReserved(sc2::Tag tag) const {
    return m_reservedUnits.find(tag) != m_reservedUnits.end();
}

std::unique_ptr<Brain> gBrain;
