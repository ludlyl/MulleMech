#include "Worker.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

Worker::Worker(const sc2::Unit& unit_):
    Unit(unit_) {
}

void Worker::BuildRefinery(Order* order_, const Unit* geyser_) {
    if (alliance == sc2::Unit::Alliance::Self) {
        order_->assignee = this;

        gAPI->action().Build(*order_, geyser_);
        m_job = Job::building;
    }
}

void Worker::Build(Order* order_, const sc2::Point2D& point_) {
    if (alliance == sc2::Unit::Alliance::Self) {
        order_->assignee = this;

        gAPI->action().Build(*order_, point_);
        m_job = Job::building;
    }
}

void Worker::Build(const Unit* building_) {
    if (alliance == sc2::Unit::Alliance::Self) {
        gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, building_);
        m_job = Job::building;
    }
}

void Worker::GatherVespene(const Unit* target_) {
    if (alliance == sc2::Unit::Alliance::Self) {
        gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, target_);
        m_job = Job::gathering_vespene;

        Units ccs = gAPI->observer().GetUnits(IsTownHall(), sc2::Unit::Alliance::Self);
        if (auto cc = ccs.GetClosestUnit(target_->pos))
            SetHomeBase(gHub->GetClosestExpansion(cc->pos));
    }
}

void Worker::SetHomeBase(std::shared_ptr<Expansion> base) {
    if (alliance == sc2::Unit::Alliance::Self) {
        m_homeBase = std::move(base);
    }
}

std::shared_ptr<Expansion> Worker::GetHomeBase() const {
    return m_homeBase;
}

void Worker::Mine() {
    if (alliance == sc2::Unit::Alliance::Self) {
        // TODO: Change this to check all mineral patches (not just the visible ones)
        auto visibleMinerals = gAPI->observer().GetUnits(IsVisibleMineralPatch(), sc2::Unit::Alliance::Neutral);

        sc2::Point2D pos;
        if (m_homeBase)
            pos = m_homeBase->town_hall_location;
        else
            pos = gAPI->observer().StartingLocation();

        auto mineralTarget = visibleMinerals.GetClosestUnit(pos);
        if (mineralTarget) {
            gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, mineralTarget);
            m_job = Job::gathering_minerals;
        }
    }
}

void Worker::SetAsUnemployed() {
    if (alliance == sc2::Unit::Alliance::Self) {
        m_job = Job::unemployed;
    }
}

void Worker::SetAsScout() {
    if (alliance == sc2::Unit::Alliance::Self) {
        m_job = Job::scouting;
    }
}

void Worker::SetAsFighter() {
    if (alliance == sc2::Unit::Alliance::Self) {
        m_job = Job::fighting;
    }
}
