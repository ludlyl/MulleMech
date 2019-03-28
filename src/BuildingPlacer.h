#pragma once

#include "core/Order.h"
#include "plugins/Plugin.h"
#include "plugins/BuildInfo.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include "Dispatcher.h"

#include <optional>

class BuildingPlacer {
public:
    static sc2::Point3D GetCenterBehindMinerals(const sc2::Point3D baseLocation);

    static std::optional<sc2::Point3D> CalculateFreePlaceBehindMinerals(const Order &order,
        const sc2::Point3D baseLocation);

    static sc2::Point3D GetPointFrontOfCC(const sc2::Point3D& baseLocation);

    static std::optional<const sc2::Point3D> FindPlaceInFrontOfCC(const Order& order, const sc2::Point3D baseLocation);

    static float GetBaseKValue();

    static const std::vector<const sc2::Unit*> GetGeysersPos();

    static sc2::Point2D getPointInLine(sc2::Point2D p1, float kValue, float dist);

    static float getKValue(sc2::Point2D p1, sc2::Point2D p2);

};
