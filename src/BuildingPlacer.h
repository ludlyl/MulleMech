#pragma once

#include "core/Order.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <optional>

class BuildingPlacer {
public:
    static sc2::Point3D GetCenterBehindMinerals(const sc2::Point3D& baseLocation);

    static std::optional<sc2::Point3D> CalculateFreePlaceBehindMinerals(const Order& order,
                                                                        const sc2::Point3D& baseLocation);
};
