// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Order.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

struct IsUnit {
    explicit IsUnit(sc2::UNIT_TYPEID type_, bool with_not_finished = false);

    bool operator()(const sc2::Unit& unit_);

 private:
    sc2::UNIT_TYPEID m_type;
    float m_build_progress;
};

// FIXME(alkurbatov): Check that the provided unit is not depleted mineral patch
struct IsMineralPatch {
    bool operator()(const sc2::Unit& unit_);
};

// Check that the provided unit is not depleted geyser
struct IsGeyser {
    bool operator()(const sc2::Unit& unit_);
};

// Check that the provided unit is not occupied and not depleted geyser
struct IsFreeGeyser {
    bool operator()(const sc2::Unit& unit_);
};

struct IsRefinery {
    bool operator()(const sc2::Unit& unit_);
};

struct IsWorker {
    bool operator()(const sc2::Unit& unit_);
};

struct IsFreeWorker {
    bool operator()(const sc2::Unit& unit_);
};

struct IsFreeLarva {
    bool operator()(const sc2::Unit& unit_);
};

struct IsGasWorker {
    bool operator()(const sc2::Unit& unit_);
};

struct IsBuildingOrder {
    bool operator()(const sc2::UnitOrder& order_);
};

struct IsCommandCenter {
    bool operator()(const sc2::Unit& unit_);
};

struct IsFreeCommandCenter {
    bool operator()(const sc2::Unit& unit_);
};

struct IsOrdered {
    explicit IsOrdered(sc2::UNIT_TYPEID type_);

    bool operator()(const Order& order_);

 private:
    sc2::UNIT_TYPEID m_type;
};
