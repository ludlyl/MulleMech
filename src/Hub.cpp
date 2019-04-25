#include <utility>

// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Hub.h"
#include "Historican.h"
#include "core/Helpers.h"
#include "IntelligenceHolder.h"

#include <algorithm>
#include <cmath>

namespace {
struct SortByDistance {
    explicit SortByDistance(const sc2::Point3D& point_):
            m_point(point_) {
    }

    bool operator()(const std::shared_ptr<Expansion>& lhs_, const std::shared_ptr<Expansion>& rhs_) const {
        return sc2::DistanceSquared2D(lhs_->town_hall_location, m_point) <
               sc2::DistanceSquared2D(rhs_->town_hall_location, m_point);
    }

 private:
    sc2::Point3D m_point;
};

}  // namespace

Construction::Construction(Unit* building_, Unit* scv_)
    : building(building_), scv(scv_) { }

Unit* Construction::GetBuilding() const {
    return building;
}

Unit* Construction::GetScvIfAlive() const {
    if (!scv->is_alive)
        return nullptr;
    return scv;
}

Hub::Hub(sc2::Race current_race_, Expansions expansions_):
    m_current_race(current_race_), m_expansions(std::move(expansions_)),
    m_current_worker_type(sc2::UNIT_TYPEID::INVALID) {
    std::sort(m_expansions.begin(), m_expansions.end(),
        SortByDistance(gAPI->observer().StartingLocation()));

    switch (m_current_race) {
        case sc2::Race::Protoss:
            m_current_worker_type = sc2::UNIT_TYPEID::PROTOSS_PROBE;
            return;

        case sc2::Race::Terran:
            m_current_worker_type = sc2::UNIT_TYPEID::TERRAN_SCV;
            return;

        case sc2::Race::Zerg:
            m_current_worker_type = sc2::UNIT_TYPEID::ZERG_DRONE;
            return;

        default:
            return;
    }
}

void Hub::OnUnitCreated(Unit* unit_) {
    // Record newly started constructions, noting which SCV is constructing it
    if (IsBuilding()(*unit_) && unit_->alliance == sc2::Unit::Alliance::Self) {
        auto buildingData = unit_->GetTypeData();

        // Find the SCV that's constructing this building
        auto scvs = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And, {IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV),
            [&unit_, &buildingData](const sc2::Unit& scv) {
                for (auto& order : scv.orders) {
                    if (order.ability_id == buildingData.ability_id) {
                        // Special case ("hack") for refineries. This is really ugly and a better solution should probably be made
                        auto pos = sc2::Distance2D(scv.pos, unit_->pos);
                        if (order.target_unit_tag != sc2::NullTag &&
                            sc2::Distance2D(scv.pos, unit_->pos) < unit_->radius + RefineryConstructionToScvExtraDistance) {
                            return true;
                        } else if (order.target_pos.x == unit_->pos.x && order.target_pos.y == unit_->pos.y) {
                            return true;
                        }
                    }
                }
                return false;
            }
        }), sc2::Unit::Alliance::Self);

        // Make (and record) a new Construction data
        if (!scvs.empty()) {
            m_constructions.emplace_back(unit_, scvs[0]);
        }
    }

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
            for (const auto& i : m_expansions) {
                if (std::floor(i->town_hall_location.x) != std::floor(unit_->pos.x) ||
                        std::floor(i->town_hall_location.y) != std::floor(unit_->pos.y))
                    continue;

                i->alliance = sc2::Unit::Alliance::Self;
                i->town_hall = unit_;

                if (i->alliance != sc2::Unit::Alliance::Self) {
                    gHistory.info() << "Captured region: (" <<
                        unit_->pos.x << ", " << unit_->pos.y <<
                        ")" << std::endl;
                }
                return;
            }
            return;

        default:
            return;
    }
}

void Hub::OnUnitDestroyed(Unit* unit_) {
    // Erase on-going construction if building was destroyed
    if (IsBuilding()(*unit_) && unit_->alliance == sc2::Unit::Alliance::Self) {
        for (auto itr = m_constructions.begin(); itr != m_constructions.end(); ++itr) {
            if (itr->building == unit_) {
                m_constructions.erase(itr);
                break;
            }
        }
    }

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
            for (const auto& i : m_expansions) {
                if (std::floor(i->town_hall_location.x) != std::floor(unit_->pos.x) ||
                        std::floor(i->town_hall_location.y) != std::floor(unit_->pos.y))
                    continue;

                i->alliance = sc2::Unit::Alliance::Neutral;
                i->town_hall = nullptr;
                gHistory.info() << "Lost region: (" <<
                    unit_->pos.x << ", " << unit_->pos.y <<
                    ")" << std::endl;
                return;
            }
            return;

        default:
            return;
    }
}

void Hub::OnUnitIdle(Unit* unit_) {
    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE: {
            // TODO: This shouldn't be handled by Hub
            auto job = unit_->AsWorker()->GetJob();
            if (job == Worker::Job::gathering_minerals ||
                job == Worker::Job::gathering_vespene ||
                job == Worker::Job::building) {
                unit_->AsWorker()->SetAsUnemployed();
                gHistory.info() << "Our busy worker has finished task" << std::endl;
            }
            break;
        }
        default:
            break;
    }
}

void Hub::OnBuildingConstructionComplete(Unit* building_) {
    // Remove finished building from our on-going constructions list
    for (auto itr = m_constructions.begin(); itr != m_constructions.end(); ++itr) {
        if (itr->building == building_) {
            // "Hack" to set the new job for scv:s finishing refinery construction
            if (itr->building->unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY) {
                itr->scv->AsWorker()->SetAsUnemployed();
            }

            m_constructions.erase(itr);
            break;
        }
    }
}

sc2::Race Hub::GetCurrentRace() const {
    return m_current_race;
}

sc2::UNIT_TYPEID Hub::GetCurrentWorkerType() const {
    return m_current_worker_type;
}

Unit* Hub::GetFreeBuildingProductionAssignee(const Order *order_, sc2::UNIT_TYPEID building_) {
    if (order_->assignee) {
        if (IsIdleUnit(order_->assignee->unit_type)(*order_->assignee)) {
            return order_->assignee;
        }
    } else {
        auto units = gAPI->observer().GetUnits(IsIdleUnit(building_), sc2::Unit::Alliance::Self);
        if (!units.empty()) {
            return units.front();
        }
    }
    return nullptr;
}

Unit* Hub::GetFreeBuildingProductionAssignee(const Order *order_, sc2::UNIT_TYPEID building_,
                                             sc2::UNIT_TYPEID addon_requirement_) {
    if (order_->assignee) {
        return GetFreeBuildingProductionAssignee(order_);
    }

    auto units = gAPI->observer().GetUnits(
            MultiFilter(MultiFilter::Selector::And, {IsIdleUnit(building_), HasAddon(addon_requirement_)}),
            sc2::Unit::Alliance::Self);
    if (!units.empty()) {
        return units.front();
    }
    return nullptr;
}

bool Hub::AssignBuildingProduction(Order* order_, Unit* assignee_) {
    if (assignee_ && IsIdleUnit(assignee_->unit_type)(*assignee_)) {
        order_->assignee = assignee_;
        return true;
    }
    return false;
}

bool Hub::AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_) {
    return AssignBuildingProduction(order_, GetFreeBuildingProductionAssignee(order_, building_));
}

bool Hub::AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_, sc2::UNIT_TYPEID addon_requirement_) {
    return AssignBuildingProduction(order_, GetFreeBuildingProductionAssignee(order_, building_, addon_requirement_));
}

const Expansions& Hub::GetExpansions() const {
    return m_expansions;
}

std::shared_ptr<Expansion> Hub::GetClosestExpansion(const sc2::Point2D& location_) const {
    assert(!m_expansions.empty());

    auto closest = m_expansions[0];
    for (auto& exp : m_expansions) {
        if (sc2::DistanceSquared2D(location_, exp->town_hall_location) <
            sc2::DistanceSquared2D(location_, closest->town_hall_location)) {
            closest = exp;
        }
    }

    return closest;
}

Expansions Hub::GetOurExpansions() const {
    Expansions expos;

    for (auto& expo : m_expansions) {
        if (expo->alliance == sc2::Unit::Alliance::Self)
            expos.push_back(expo);
    }

    // Sort bases by how far they are (walkable distance) from the main, with the assumption
    // that such a sorting will tell us which base is the natural, and so forth
    auto starting = GetClosestExpansion(gAPI->observer().StartingLocation());
    std::sort(expos.begin(), expos.end(), [&starting](auto& e1, auto& e2) {
        return starting->distanceTo(e1) < starting->distanceTo(e2);
    });

    return expos;
}

int Hub::GetOurExpansionCount() const {
    int count = 0;
    for (auto& expo : gHub->GetExpansions()) {
        if (expo->alliance == sc2::Unit::Alliance::Self) {
            count++;
        }
    }
    return count;
}

std::unique_ptr<Hub> gHub;
