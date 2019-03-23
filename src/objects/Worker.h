// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "GameObject.h"
#include "core/Order.h"

enum Job {
    GATHERING_MINERALS = 0,
    GATHERING_VESPENE = 1,
    BUILDING = 2,
    BUILDING_REFINERY = 3,
};

struct Worker : GameObject {
    explicit Worker(const Unit& unit_);

    void BuildRefinery(Order* order_, const Unit& geyser_);

    void Build(Order* order_, const sc2::Point2D& point_);

    void GatherVespene(const Unit& target_);

 private:
    Job m_job;
};
