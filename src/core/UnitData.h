#pragma once

#include <sc2api/sc2_unit.h>

#include <memory>

class MicroPlugin;

struct UnitData {
    UnitData(const sc2::Unit* unit_);
    ~UnitData();
    const sc2::Unit* unit;
    std::unique_ptr<MicroPlugin> micro;
};
