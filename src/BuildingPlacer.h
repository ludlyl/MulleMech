#pragma once

#include "core/Order.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <optional>
#include <core/Units.h>

class BuildingPlacer {
public:
    static sc2::Point3D GetCenterBehindMinerals(const sc2::Point3D& baseLocation);

    static std::optional<sc2::Point3D> CalculateFreePlaceBehindMinerals(const Order& order,
                                                                        const sc2::Point3D& baseLocation);
    // Calculates point in front of HQ (i.e. CC), away from mineral line
    static sc2::Point3D GetPointInFrontOfTownHall(const sc2::Point3D& baseLocation);

    // Returns a random building point through a line in the point in front of HQ (i.e. CC)
    static std::optional<sc2::Point3D> CalculateFreePlaceInFrontOfTownHall(const Order& order,
                                                                           const sc2::Point3D& baseLocation,
                                                                           bool includeSpaceForAddon = false);

private:
    static float GetBaseKValue(const sc2::Point3D& baseLocation);

    static Units GetVisibleMineralPatchesAtBase(const sc2::Point3D& baseLocation);

    static Units GetVisibleGeysersAtBase(const sc2::Point3D& baseLocation);

    static sc2::Point2D GetPointInLine(const sc2::Point2D& p1, float kValue, float dist);

    static float GetKValue(const sc2::Point2D& p1, const sc2::Point2D& p2);
};
