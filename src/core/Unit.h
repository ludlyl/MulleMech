#pragma once

#include <sc2api/sc2_unit.h>
#include <memory>

class MicroPlugin;
struct UnitData;

class Unit {
public:
    explicit Unit(std::shared_ptr<UnitData> data);

    // Implicit bi-directional conversion: Unit <-> const sc2::Unit*, and
    // implicit uni-directional conversion Unit -> const sc2::Unit&
    Unit(const sc2::Unit* unit);
    operator const sc2::Unit&() const;
    operator const sc2::Unit*() const;

    // Dereference to get sc2::Unit access
    const sc2::Unit* operator->() const;
    const sc2::Unit& operator*() const;

    // Micro plugin for this unit
    void InstallMicro();
    MicroPlugin* Micro() const;

private:
    std::shared_ptr<UnitData> m_data;
};
