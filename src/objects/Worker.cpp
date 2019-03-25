// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Worker.h"
#include "core/API.h"

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
