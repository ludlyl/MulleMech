// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Order.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

constexpr float F_PI = 3.1415927f;
constexpr float F_2PI = 2.0f * 3.1415927f;

struct IsUnit {
    explicit IsUnit(sc2::UNIT_TYPEID type_, bool with_not_finished = false);

    bool operator()(const sc2::Unit& unit_) const;

 private:
    sc2::UNIT_TYPEID m_type;
    float m_build_progress;
};

struct IsCombatUnit {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsBuilding {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsVisibleMineralPatch {
    // NOTE (alkurbatov): All the visible mineral patches has non-zero mineral
    // contents while the mineral patches covered by the fog of war don't have
    // such parameter (it is always zero) and can't be selected/targeted.
    // This filter returns only the visible  and not depleted mineral patches.

    bool operator()(const sc2::Unit& unit_) const;
};

struct IsFoggyResource {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsVisibleGeyser {
    // NOTE (alkurbatov): All the geysers has non-zero vespene contents while
    // the geysers covered by the fog of war don't have such parameter
    // (it is always zero) and can't be selected/targeted.
    // This filter returns only the visible  and not depleted geysers.

    bool operator()(const sc2::Unit& unit_) const;
};

// Check that the provided unit is not occupied and not depleted geyser
struct IsFreeGeyser {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsRefinery {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsIdleUnit {
    explicit IsIdleUnit(sc2::UNIT_TYPEID type_);

    bool operator()(const sc2::Unit& unit_) const;

 private:
    sc2::UNIT_TYPEID m_type;
};

struct IsWorker {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsGasWorker {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsTownHall {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsIdleTownHall {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsCommandCenter {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsOrdered {
    explicit IsOrdered(sc2::UNIT_TYPEID type_);

    bool operator()(const Order& order_) const;

 private:
    sc2::UNIT_TYPEID m_type;
};

// These should maybe be public on be placed somewhere else
static constexpr float ADDON_DISPLACEMENT_IN_X = 2.5f;
static constexpr float ADDON_DISPLACEMENT_IN_Y = -0.5f;

sc2::Point2D GetTerranAddonPosition(const sc2::Unit& unit_);

struct ClosestToPoint2D {
    explicit ClosestToPoint2D(sc2::Point2D point) : m_point(point) {

    }

    bool operator()(const sc2::Point2D& a, const sc2::Point2D& b) const {
        return sc2::DistanceSquared2D(m_point, a) < sc2::DistanceSquared2D(m_point, b);
    }

private:
    sc2::Point2D m_point;
};

std::vector<sc2::Point2D> PointsInCircle(float radius, const sc2::Point2D& center, int numPoints = 12);

std::vector<sc2::Point2D> PointsInCircle(float radius, const sc2::Point2D& center, float forcedHeight, int numPoints = 12);
