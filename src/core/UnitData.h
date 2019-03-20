#pragma once

#include "plugins/micro/MicroPlugin.h"
#include <sc2api/sc2_unit.h>
#include <memory>

struct UnitData {
    explicit UnitData(const sc2::Unit* unit_);
    ~UnitData() = default;
    const sc2::Unit* unit;
    std::unique_ptr<MicroPlugin> micro;
};
