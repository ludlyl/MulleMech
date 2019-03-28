// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Unit.h"
#include "core/Order.h"

struct Expansion;

enum Job {
    GATHERING_MINERALS = 0,
    GATHERING_VESPENE = 1,
    BUILDING = 2,
    BUILDING_REFINERY = 3,
};

class Worker : public Unit {
public:
    explicit Worker(const sc2::Unit& unit_);

    void BuildRefinery(Order* order_, const Unit* geyser_);

    void Build(Order* order_, const sc2::Point2D& point_);

    void GatherVespene(const Unit* target_);

    void SetHomeBase(std::shared_ptr<Expansion> base);

    std::shared_ptr<Expansion> GetHomeBase() const;

    // Make SCV go back to gathering minerals
    void Mine();

    Job GetJob() const { return m_job; }

private:
    Job m_job;
    std::shared_ptr<Expansion> m_homeBase;
};
