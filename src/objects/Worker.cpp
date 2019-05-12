#include "Worker.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

Worker::Worker(const sc2::Unit& unit_):
    Unit(unit_) {
}

void Worker::BuildRefinery(Order* order_, const Unit* geyser_) {
    assert(alliance == sc2::Unit::Alliance::Self);
    order_->assignee = this;
    gAPI->action().Build(*order_, geyser_);
    // As the worker will go over to collecting gas as fast as the building is completed,
    // settings it's job to "building" here is a bit problematic. The way we currently fix
    // this is by calling SetAsUnemployed on workers that finishes refineries in Hub::OnBuildingConstructionComplete
    // We should find a better way to solve this!
    m_job = Job::building;
}

void Worker::Build(Order* order_, const sc2::Point2D& point_) {
    assert(alliance == sc2::Unit::Alliance::Self);
    order_->assignee = this;
    gAPI->action().Build(*order_, point_);
    m_job = Job::building;
}

void Worker::Build(const Unit* building_) {
    assert(alliance == sc2::Unit::Alliance::Self);
    gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, building_);
    m_job = Job::building;
}

void Worker::Repair(const Unit* target_) {
    // TODO: Assert here if the target unit doesn't have the mechanical attribute
    assert(alliance == sc2::Unit::Alliance::Self);
    gAPI->action().Cast(this, sc2::ABILITY_ID::EFFECT_REPAIR, target_);
    m_job = Job::repairing;
}

void Worker::GatherVespene(const Unit* target_) {
    assert(alliance == sc2::Unit::Alliance::Self);
    gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, target_);
    m_job = Job::gathering_vespene;

    Units ccs = gAPI->observer().GetUnits(IsTownHall(), sc2::Unit::Alliance::Self);
    if (auto cc = ccs.GetClosestUnit(target_->pos))
        SetHomeBase(gHub->GetClosestExpansion(cc->pos));
}

void Worker::SetHomeBase(std::shared_ptr<Expansion> base) {
    assert(alliance == sc2::Unit::Alliance::Self);
    m_home_base = std::move(base);
}

std::shared_ptr<Expansion> Worker::GetHomeBase() const {
    return m_home_base;
}

void Worker::Mine() {
    assert(alliance == sc2::Unit::Alliance::Self);
    if (m_home_base) {
        assert(m_home_base->alliance == sc2::Unit::Alliance::Self &&
               m_home_base->town_hall && m_home_base->town_hall->is_alive);
    }

    // Calculate new home base if the worker doesn't have one or if the current one doesn't have any minerals left
    if (!m_home_base || m_home_base->town_hall->ideal_harvesters == 0) {
        float distance_to_closest_town_hall_with_minerals = std::numeric_limits<float>::max();
        const std::shared_ptr<Expansion>* closest_base_with_minerals = nullptr;
        for (auto& expansion : gHub->GetExpansions()) {
            if (expansion->alliance == sc2::Unit::Alliance::Self && expansion->town_hall->ideal_harvesters > 0) {
                float distance_to_town_hall = sc2::DistanceSquared2D(this->pos,  expansion->town_hall->pos);
                if (distance_to_town_hall < distance_to_closest_town_hall_with_minerals) {
                    distance_to_closest_town_hall_with_minerals = distance_to_town_hall;
                    closest_base_with_minerals = &expansion;
                }
            }
        }

        if (closest_base_with_minerals) {
            m_home_base = *closest_base_with_minerals;
        }
    }

    if (m_home_base) {
        auto mineral_patches = gAPI->observer().GetUnits(IsMineralPatch(), sc2::Unit::Alliance::Neutral);
        auto mineral_target = mineral_patches.GetClosestUnit(m_home_base->town_hall->pos);

        if (mineral_target) {
            gAPI->action().Cast(this, sc2::ABILITY_ID::SMART, mineral_target);
            m_job = Job::gathering_minerals;
        }
    }
}

void Worker::SetAsUnemployed() {
    assert(alliance == sc2::Unit::Alliance::Self);
    m_job = Job::unemployed;
}

void Worker::SetAsScout() {
    assert(alliance == sc2::Unit::Alliance::Self);
    m_job = Job::scouting;
}

void Worker::SetAsFighter() {
    assert(alliance == sc2::Unit::Alliance::Self);
    m_job = Job::fighting;
}
