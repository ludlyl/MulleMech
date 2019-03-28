// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Worker.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

Worker::Worker(const sc2::Unit& unit_):
    Unit(unit_), m_job(Job::GATHERING_MINERALS) {
}

void Worker::BuildRefinery(Order* order_, const Unit* geyser_) {
    order_->assignee = this;

    gAPI->action().Build(*order_, geyser_);
    m_job = Job::BUILDING_REFINERY;
}

void Worker::Build(Order* order_, const sc2::Point2D& point_) {
    order_->assignee = this;

    gAPI->action().Build(*order_, point_);
    m_job = Job::BUILDING;
}

void Worker::GatherVespene(const Unit* target_) {
    gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, target_);
    m_job = Job::GATHERING_VESPENE;
}

void Worker::SetHomeBase(std::shared_ptr<Expansion> base) {
    m_homeBase = base;
}

std::shared_ptr<Expansion> Worker::GetHomeBase() const {
    return m_homeBase;
}

void Worker::Mine() {
    auto units = gAPI->observer().GetUnits(IsVisibleMineralPatch(), sc2::Unit::Alliance::Neutral);

    sc2::Point2D pos;
    if (m_homeBase)
        pos = m_homeBase->town_hall_location;
    else
        pos = gAPI->observer().StartingLocation();

    auto mineral_target = units.GetClosestUnit(pos);
    if (mineral_target) {
        gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, mineral_target);
        m_job = GATHERING_MINERALS;
    }
}
