#pragma once

#include "core/Order.h"
#include "plugins/Plugin.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <Dispatcher.h>

#include <optional>

struct BuildingPlacer : Plugin {
    void OnGameStart(Builder*) final;

    BuildingPlacer();

    static sc2::Point3D GetCenterBehindMinerals(const sc2::Point3D baseLocation);

    static std::optional<sc2::Point3D> CalculateFreePlaceBehindMinerals(const Order &order,
        const sc2::Point3D baseLocation);

    static sc2::Point3D GetPointFrontOfCC(const sc2::Point3D baseLocation);

    static std::optional<sc2::Point3D> FindPlaceInFrontOfCC(const Order& order, const sc2::Point3D baseLocation);

    static float GetBaseKValue();

    static const std::vector<const sc2::Unit*> getGeysersPos();

    static sc2::Point2D getPointInLine(sc2::Point2D p1, float kValue, float dist);

    static float getKValue(sc2::Point2D p1, sc2::Point2D p2);

private:
    static BuildingPlacer* m;

    const sc2::ObservationInterface* m_observer;
};

static sc2::Point3D buildingPoint;

static float baseKValue;
