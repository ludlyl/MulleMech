// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Order.h"
#include "core/Unit.h"
#include "objects/Worker.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <functional>
#include <initializer_list>
#include <vector>

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

struct IsTemporaryUnit {
    bool operator()(const sc2::Unit& unit_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

// Anti air = Can the unit attack air at all
struct IsAntiAirUnit {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsBuilding {
    bool operator()(const sc2::Unit& unit_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

// I.e. is it a barracks, factory or starport
struct IsBuildingWithSupportForAddon {
    bool operator()(const sc2::Unit& type_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

struct IsAddon {
    bool operator()(const sc2::Unit& type_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

struct IsVisibleMineralPatch {
    // NOTE (alkurbatov): All the visible mineral patches has non-zero mineral
    // contents while the mineral patches covered by the fog of war don't have
    // such parameter (it is always zero) and can't be selected/targeted.
    // This filter returns only the visible  and not depleted mineral patches.

    bool operator()(const sc2::Unit& unit_) const;
};

struct IsMineralPatch {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsGeyser {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsVisibleUndepletedGeyser {
    // NOTE (alkurbatov): All the geysers has non-zero vespene contents while
    // the geysers covered by the fog of war don't have such parameter
    // (it is always zero) and can't be selected/targeted.
    // This filter returns only the visible  and not depleted geysers.

    bool operator()(const sc2::Unit& unit_) const;
};

struct IsFoggyResource {
    bool operator()(const sc2::Unit& unit_) const;
};

// I.e. IsFinishedRefinery
struct IsRefinery {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsIdleUnit {
    explicit IsIdleUnit(sc2::UNIT_TYPEID type_, bool count_non_full_reactor_as_idle_ = true);

    bool operator()(const sc2::Unit& unit_) const;

 private:
    sc2::UNIT_TYPEID m_type;
    bool m_count_non_full_reactor_as_idle;
};

struct IsWorker {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsWorkerWithJob {
    explicit IsWorkerWithJob(Worker::Job job_);

    bool operator()(const sc2::Unit& unit_) const;

private:
    Worker::Job m_job;
};

struct IsTownHall {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsIdleTownHall {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsOrdered {
    explicit IsOrdered(sc2::UNIT_TYPEID type_);

    bool operator()(const Order& order_) const;

 private:
    sc2::UNIT_TYPEID m_type;
};

struct IsWithinDist {
    explicit IsWithinDist(const sc2::Point3D& center, float dist_) : m_center(center), m_distSq(dist_ * dist_), m_2d(false) { }
    explicit IsWithinDist(const sc2::Point2D& center, float dist_) :
        m_center(sc2::Point3D(center.x, center.y, 0)), m_distSq(dist_ * dist_), m_2d(true) { }

    bool operator()(const sc2::Unit& unit_) const;

private:
    sc2::Point3D m_center;
    float m_distSq;
    bool m_2d;
};

// Send in sc2::UNIT_TYPEID::INVALID to check if the building doesn't have an add-on
struct HasAddon {
    explicit HasAddon(sc2::UNIT_TYPEID addon_type_);

    bool operator()(const sc2::Unit& unit_) const;

private:
    sc2::UNIT_TYPEID m_addon_type;
};

struct MultiFilter {
    enum class Selector {
        And,
        Or
    };

    MultiFilter(Selector selector, std::initializer_list<std::function<bool(const sc2::Unit& unit)>> fns_);

    bool operator()(const sc2::Unit& unit_) const;

private:
    std::vector<std::function<bool(const sc2::Unit& unit)>> m_functors;
    Selector m_selector;
};

struct Inverse {
    explicit Inverse(std::function<bool(const sc2::Unit& unit)> functor);

    bool operator()(const sc2::Unit& unit_) const;

private:
    std::function<bool(const sc2::Unit& unit)> m_functor;
};

// These should maybe be public on be placed somewhere else
static constexpr float ADDON_DISPLACEMENT_IN_X = 2.5f;
static constexpr float ADDON_DISPLACEMENT_IN_Y = -0.5f;

sc2::Point2D GetTerranAddonPosition(const Unit* unit_);
sc2::Point2D GetTerranAddonPosition(const sc2::Point2D& parentBuildingPosition);

struct ClosestToPoint2D {
    explicit ClosestToPoint2D(sc2::Point2D point) : m_point(point) {

    }

    bool operator()(const sc2::Point2D& a, const sc2::Point2D& b) const {
        return sc2::DistanceSquared2D(m_point, a) < sc2::DistanceSquared2D(m_point, b);
    }

private:
    sc2::Point2D m_point;
};

struct CloakState {
    explicit CloakState(sc2::Unit::CloakState state_) : m_state(state_) { }
    bool operator()(const sc2::Unit& unit_) const;

private:
    sc2::Unit::CloakState m_state;
};

std::vector<sc2::Point2D> PointsInCircle(float radius, const sc2::Point2D& center, int numPoints = 12);

std::vector<sc2::Point2D> PointsInCircle(float radius, const sc2::Point2D& center, float forcedHeight, int numPoints = 12);

sc2::Point2D Rotate2D(sc2::Point2D vector, float rotation);

// Returns "all" the (correct) tech requirements needed for a unit type.
// This is needed as UnitTypeData only can hold one requirement and some units have more than one
// (e.g. Thors requires both an armory and a FACTORYTECHLAB). Furthermore this is needed as UnitTypeData doesn't
// specify the type of techlab.
std::vector<sc2::UnitTypeID> GetAllTechRequirements(sc2::UnitTypeID id_);

std::vector<sc2::UnitTypeID> GetAllTechRequirements(const sc2::UnitTypeData& data_);

std::vector<sc2::UnitTypeID> GetAllTechRequirements(sc2::AbilityID id_, sc2::UnitTypeID suppliedTechRequirement_ = sc2::UNIT_TYPEID::INVALID);

// I.e. get mining and unemployed workers
Units GetFreeWorkers();

Worker* GetClosestFreeWorker(const sc2::Point2D& location_);

bool FreeWorkerExists();
