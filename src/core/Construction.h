#pragma once

#include "Unit.h"

// Represents both a construction order and an ongoing construction
struct Construction {
    Construction(sc2::Point3D position_, sc2::UNIT_TYPEID building_type_, bool includes_add_on_space_ = false)
            : position(position_), building_type(building_type_), includes_add_on_space(includes_add_on_space_) {}

    sc2::Point3D position;
    sc2::UNIT_TYPEID building_type;
    bool includes_add_on_space;
    Unit* building = nullptr;
};
