// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Unit.h"
#include "core/Map.h"
#include "objects/Worker.h"

#include <sc2api/sc2_gametypes.h>
#include <sc2api/sc2_unit.h>

#include <functional>
#include <list>
#include <memory>
#include <optional>

struct Hub {
    Hub(sc2::Race current_race_, Expansions  expansions_);

    void OnUnitCreated(Unit* unit_);

    void OnUnitDestroyed(Unit* unit_);

    sc2::Race GetCurrentRace() const;

    sc2::UNIT_TYPEID GetCurrentWorkerType() const;

    // Returns nullptr if no building to produce Units/Upgrades/Addons/Mutations from/on is available
    Unit* GetFreeBuildingProductionAssignee(const Order *order_, sc2::UNIT_TYPEID building_ = sc2::UNIT_TYPEID::INVALID);

    // If INVALID is sent in as a addon_requirement (and no assignee is provided) the order is assigned to a unit with no add-on
    Unit* GetFreeBuildingProductionAssignee(const Order *order_, sc2::UNIT_TYPEID building_,
                                            sc2::UNIT_TYPEID addon_requirement_);

    // This should always be used instead of manually setting the assignee for production
    bool AssignBuildingProduction(Order* order_, Unit* assignee_);

    // Find first free building to produce Units/Upgrades/Addons/Mutations from/on
    bool AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_ = sc2::UNIT_TYPEID::INVALID);

    // If INVALID is sent in as a addon_requirement (and no assignee is provided) the order is assigned to a unit with no add-on
    bool AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_, sc2::UNIT_TYPEID addon_requirement_);

    // Sorted by distance from our starting location
    const Expansions& GetExpansions() const;

    std::shared_ptr<Expansion> GetClosestExpansion(const sc2::Point2D& location_) const;

    // Returns a list of our expansions sorted with walking distance to starting location
    // index: 0=>main base, 1=>natural, etc
    Expansions GetOurExpansions() const;

    int GetOurExpansionCount() const;

    // Request scan from Orbital Command
    void RequestScan(const sc2::Point2D& pos);

 private:
    sc2::Race m_current_race;
    Expansions m_expansions;
    sc2::UNIT_TYPEID m_current_worker_type;

    uint32_t m_lastStepScan;

    // This is a very arbitrary number used for checking if an scv is currently constructing a specific refinery
    // (by taking the refinery's position + its radius + this value and seeing if the scv is inside that distance)
    static constexpr float RefineryConstructionToScvExtraDistance = 0.25f;
};

extern std::unique_ptr<Hub> gHub;
