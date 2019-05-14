// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Order.h"
#include "core/Unit.h"
#include "objects/Worker.h"
#include "API.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <functional>
#include <initializer_list>
#include <vector>

constexpr float F_PI = 3.1415927f;
constexpr float F_2PI = 2.0f * 3.1415927f;

struct IsUnit {
    explicit IsUnit(sc2::UNIT_TYPEID type_, bool with_not_finished_ = false);

    bool operator()(const Unit* unit_) const;

 private:
    sc2::UNIT_TYPEID m_type;
    float m_build_progress;
};

struct IsDamaged {
    bool operator()(const Unit* unit_) const;
};

// Includes "combat buildings" (cannon, turrets etc.)
struct IsCombatUnit {
    bool operator()(const Unit* unit_) const;
};

struct IsTemporaryUnit {
    bool operator()(const Unit* unit_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

// Anti air = Can the unit attack air at all
struct IsAntiAirUnit {
    bool operator()(const Unit* unit_) const;
};

struct IsBuilding {
    bool operator()(const Unit* unit_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

struct IsFinishedBuilding {
    bool operator()(const Unit* unit_) const;
};

struct IsUnfinishedBuilding {
    bool operator()(const Unit* unit_) const;
};

// I.e. is it a barracks, factory or starport
struct IsBuildingWithSupportForAddon {
    bool operator()(const Unit* type_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

struct IsAddon {
    bool operator()(const Unit* type_) const;
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

struct IsVisibleMineralPatch {
    // NOTE (alkurbatov): All the visible mineral patches has non-zero mineral
    // contents while the mineral patches covered by the fog of war don't have
    // such parameter (it is always zero) and can't be selected/targeted.
    // This filter returns only the visible  and not depleted mineral patches.

    bool operator()(const Unit* unit_) const;
};

struct IsMineralPatch {
    bool operator()(const Unit* unit_) const;
};

struct IsGeyser {
    bool operator()(const Unit* unit_) const;
};

struct IsVisibleUndepletedGeyser {
    // NOTE (alkurbatov): All the geysers has non-zero vespene contents while
    // the geysers covered by the fog of war don't have such parameter
    // (it is always zero) and can't be selected/targeted.
    // This filter returns only the visible  and not depleted geysers.

    bool operator()(const Unit* unit_) const;
};

struct IsFoggyResource {
    bool operator()(const Unit* unit_) const;
};

struct IsRefinery {
    bool operator()(const Unit* unit_) const;
};

struct IsIdleUnit {
    explicit IsIdleUnit(sc2::UNIT_TYPEID type_, bool count_non_full_reactor_as_idle_ = true);

    bool operator()(const Unit* unit_) const;

 private:
    sc2::UNIT_TYPEID m_type;
    bool m_count_non_full_reactor_as_idle;
};

struct IsWorker {
    bool operator()(const Unit* unit_) const;
};

struct IsWorkerWithJob {
    explicit IsWorkerWithJob(Worker::Job job_);

    bool operator()(const Unit* unit_) const;

private:
    Worker::Job m_job;
};

struct IsWorkerWithHomeBase {
    explicit IsWorkerWithHomeBase(const std::shared_ptr<Expansion>& home_base_);

    bool operator()(const Unit* unit_) const;

private:
    const std::shared_ptr<Expansion>& m_home_base;
};

struct IsWorkerWithUnstartedConstructionOrderFor {
    explicit IsWorkerWithUnstartedConstructionOrderFor(sc2::UNIT_TYPEID type_);

    bool operator()(const Unit* unit_) const;

private:
    sc2::UNIT_TYPEID m_type;
};

struct IsTownHall {
    bool operator()(const Unit* unit_) const;
};

struct IsIdleTownHall {
    bool operator()(const Unit* unit_) const;
};

struct IsOrdered {
    explicit IsOrdered(sc2::UNIT_TYPEID type_);

    bool operator()(const Order& order_) const;

 private:
    sc2::UNIT_TYPEID m_type;
};

struct IsWithinDist {
    explicit IsWithinDist(const sc2::Point3D& center_, float dist_) : m_center(center_), m_distSq(dist_ * dist_), m_2d(false) { }
    explicit IsWithinDist(const sc2::Point2D& center_, float dist_) :
        m_center(sc2::Point3D(center_.x, center_.y, 0)), m_distSq(dist_ * dist_), m_2d(true) { }

    bool operator()(const Unit* unit_) const;

private:
    sc2::Point3D m_center;
    float m_distSq;
    bool m_2d;
};

// Send in sc2::UNIT_TYPEID::INVALID to check if the building doesn't have an add-on
struct HasAddon {
    explicit HasAddon(sc2::UNIT_TYPEID addon_type_);

    bool operator()(const Unit* unit_) const;

private:
    sc2::UNIT_TYPEID m_addon_type;
};

struct MultiFilter {
    enum class Selector {
        And,
        Or
    };

    MultiFilter(Selector selector_, std::initializer_list<API::Filter> filters_);

    bool operator()(const Unit* unit_) const;

private:
    std::vector<API::Filter> m_filters;
    Selector m_selector;
};

struct Inverse {
    explicit Inverse(API::Filter filters_);

    bool operator()(const Unit* unit_) const;

private:
    API::Filter m_filter;
};

// These should maybe be public on be placed somewhere else
static constexpr float ADDON_DISPLACEMENT_IN_X = 2.5f;
static constexpr float ADDON_DISPLACEMENT_IN_Y = -0.5f;

sc2::Point2D GetTerranAddonPosition(const Unit* unit_);
sc2::Point2D GetTerranAddonPosition(const sc2::Point2D& parent_building_position_);

struct ClosestToPoint2D {
    explicit ClosestToPoint2D(sc2::Point2D point_) : m_point(point_) {

    }

    bool operator()(const sc2::Point2D& a_, const sc2::Point2D& b_) const {
        return sc2::DistanceSquared2D(m_point, a_) < sc2::DistanceSquared2D(m_point, b_);
    }

    bool operator()(const sc2::Unit& a_, const sc2::Unit& b_) const {
        return sc2::DistanceSquared2D(m_point, a_.pos) < sc2::DistanceSquared2D(m_point, b_.pos);
    }

    bool operator()(const Unit* a_, const Unit* b_) const {
        return sc2::DistanceSquared2D(m_point, a_->pos) < sc2::DistanceSquared2D(m_point, b_->pos);
    }

private:
    sc2::Point2D m_point;
};

struct CloakState {
    explicit CloakState(sc2::Unit::CloakState state_) : m_state(state_) { }
    bool operator()(const Unit* unit_) const;

private:
    sc2::Unit::CloakState m_state;
};

bool IsThereTooManyEnemiesToBuildAt(const sc2::Point2D& pos_);

std::vector<sc2::Point2D> PointsInCircle(float radius_, const sc2::Point2D& center_, int num_points_ = 12);

std::vector<sc2::Point2D> PointsInCircle(float radius_, const sc2::Point2D& center_, float forced_height_, int num_points_ = 12);

sc2::Point2D Rotate2D(sc2::Point2D vector_, float rotation_);

// Returns "all" the (correct) strucutre tech requirements needed for a unit type.
// This is needed as UnitTypeData only can hold one requirement and some units have more than one
// (e.g. Thors requires both an armory and a FACTORYTECHLAB). Furthermore this is needed as UnitTypeData doesn't
// specify the type of techlab.
std::vector<sc2::UnitTypeID> GetAllStructureTechRequirements(sc2::UnitTypeID id_);

std::vector<sc2::UnitTypeID> GetAllStructureTechRequirements(const sc2::UnitTypeData& data_);

std::vector<sc2::UnitTypeID> GetAllStructureTechRequirements(sc2::AbilityID id_,
                                                             sc2::UnitTypeID supplied_tech_requirements_ = sc2::UNIT_TYPEID::INVALID);

// Returns the upgrade tech requirement for a given ability.
// This is needed as e.g. bio weapons lvl 2 has a requirement on bio weapons lvl 1
sc2::UPGRADE_ID GetUpgradeTechRequirement(sc2::AbilityID id_);

// I.e. get mining and unemployed workers
Units GetFreeWorkers(bool include_gas_workers_ = false);

Worker* GetClosestFreeWorker(const sc2::Point2D& location_, bool include_gas_workers_ = false);

bool FreeWorkerExists(bool include_gas_workers_ = false);
