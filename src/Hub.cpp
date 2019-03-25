// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Historican.h"
#include "Hub.h"
#include "core/Helpers.h"

#include <algorithm>
#include <cmath>

namespace {
struct SortByDistance {
    explicit SortByDistance(const sc2::Point3D& point_);

    bool operator()(const std::shared_ptr<Expansion>& lhs_, const std::shared_ptr<Expansion>& rhs_) const;

 private:
    sc2::Point3D m_point;
};

SortByDistance::SortByDistance(const sc2::Point3D& point_):
    m_point(point_) {
}

bool SortByDistance::operator()(const std::shared_ptr<Expansion>& lhs_, const std::shared_ptr<Expansion>& rhs_) const {
    return sc2::DistanceSquared2D(lhs_->town_hall_location, m_point) <
        sc2::DistanceSquared2D(rhs_->town_hall_location, m_point);
}

}  // namespace

Construction::Construction(Unit* building_, Unit* scv_)
    : building(building_), scv(scv_) { }

Unit* Construction::GetBuilding() const {
    return building;
}

Unit* Construction::GetScv() const {
    if (!scv->is_alive)
        return nullptr;
    return scv;
}

Hub::Hub(sc2::Race current_race_, const Expansions& expansions_):
    m_current_race(current_race_), m_expansions(expansions_),
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

void Hub::OnStep() {
    m_assignedBuildings.clear();
}

void Hub::OnUnitCreated(Unit* unit_) {
    // Record newly started constructions, noting which SCV is constructing it
    if (IsBuilding()(*unit_) && unit_->alliance == sc2::Unit::Alliance::Self) {
        auto buildingData = gAPI->observer().GetUnitTypeData(unit_->unit_type);

        // Find the SCV that's constructing this building
        auto scvs = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And, {IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV),
            [&unit_, &buildingData](const sc2::Unit& scv) {
                for (auto& order : scv.orders) {
                    if (order.ability_id == buildingData.ability_id && order.target_pos.x == unit_->pos.x &&
                        order.target_pos.y == unit_->pos.y) {
                        return true;
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
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
            m_free_workers.Add(unit_->AsWorker());
            return;

        case sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR:
        case sc2::UNIT_TYPEID::TERRAN_REFINERY:
        case sc2::UNIT_TYPEID::ZERG_EXTRACTOR: {
            // Remove claimed geyser
            if (m_captured_geysers.RemoveOccupied(unit_))
                gHistory.info() << "Release claimed geyser " << std::endl;

            m_captured_geysers.Add(unit_);
            gHistory.info() << "Capture object " <<
                sc2::UnitTypeToName(unit_->unit_type) << std::endl;
            return;
        }

        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
            for (const auto& i : m_expansions) {
                if (std::floor(i->town_hall_location.x) != std::floor(unit_->pos.x) ||
                        std::floor(i->town_hall_location.y) != std::floor(unit_->pos.y))
                    continue;

                if (i->alliance == sc2::Unit::Alliance::Self)
                    return;

                i->alliance = sc2::Unit::Alliance::Self;
                gHistory.info() << "Captured region: (" <<
                    unit_->pos.x << ", " << unit_->pos.y <<
                    ")" << std::endl;
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
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE: {
            if (m_busy_workers.Remove(unit_->AsWorker())) {
                gHistory.info() << "Our busy worker was destroyed" << std::endl;
                return;
            }

            m_free_workers.Remove(unit_->AsWorker());
            return;
        }

        case sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR:
        case sc2::UNIT_TYPEID::TERRAN_REFINERY:
        case sc2::UNIT_TYPEID::ZERG_EXTRACTOR: {
            if (m_captured_geysers.Remove(unit_)) {
                gHistory.info() << "Release object " <<
                    sc2::UnitTypeToName(unit_->unit_type) << std::endl;
            }

            return;
        }

        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
            for (const auto& i : m_expansions) {
                if (std::floor(i->town_hall_location.x) != std::floor(unit_->pos.x) ||
                        std::floor(i->town_hall_location.y) != std::floor(unit_->pos.y))
                    continue;

                i->alliance = sc2::Unit::Alliance::Neutral;
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
            if (m_free_workers.Swap(unit_->AsWorker(), m_busy_workers))
                gHistory.info() << "Our busy worker has finished task" << std::endl;
            return;
        }

        default:
            break;
    }
}

void Hub::OnBuildingConstructionComplete(Unit* building_) {
    // Remove finished building from our on-going constructions list
    for (auto itr = m_constructions.begin(); itr != m_constructions.end(); ++itr) {
        if (itr->building == building_) {
            m_constructions.erase(itr);
            break;
        }
    }
}

bool Hub::IsOccupied(const Unit* unit_) const {
    return m_captured_geysers.IsOccupied(unit_);
}

bool Hub::IsTargetOccupied(const sc2::UnitOrder& order_) const {
    if (auto unit = gAPI->observer().GetUnit(order_.target_unit_tag))
        return IsOccupied(unit);
    return false;
}

void Hub::ClaimObject(Unit* unit_) {
    if (IsVisibleGeyser()(*unit_)) {
        m_captured_geysers.Add(unit_);
        gHistory.info() << "Claim object " <<
            sc2::UnitTypeToName(unit_->unit_type) << std::endl;
    }
}

sc2::Race Hub::GetCurrentRace() const {
    return m_current_race;
}

Worker* Hub::GetClosestFreeWorker(const sc2::Point2D& location_) {
    Worker* closest_worker = m_free_workers.GetClosestTo(location_);
    if (!closest_worker)
        return nullptr;

    return closest_worker;
}

bool Hub::FreeWorkerExists() {
    return !m_free_workers.Empty();
}

int Hub::GetNumberOfFreeWorkers() {
    return static_cast<int>(m_free_workers.Count());
}

sc2::UNIT_TYPEID Hub::GetCurrentWorkerType() const {
    return m_current_worker_type;
}

bool Hub::AssignRefineryConstruction(Order* order_, Unit* geyser_) {
    Worker* worker = GetClosestFreeWorker(geyser_->pos);
    if (!worker)
        return false;

    m_free_workers.Swap(worker, m_busy_workers);

    m_busy_workers.Back()->BuildRefinery(order_, geyser_);
    ClaimObject(geyser_);
    return true;
}

bool Hub::AssignBuildTask(Order* order_, const sc2::Point2D& point_) {
    Worker* worker = GetClosestFreeWorker(point_);
    if (!worker)
        return false;

    m_free_workers.Swap(worker, m_busy_workers);

    m_busy_workers.Back()->Build(order_, point_);
    return true;
}

void Hub::AssignVespeneHarvester(const Unit* refinery_) {
    Worker* worker = GetClosestFreeWorker(refinery_->pos);
    if (!worker)
        return;

    m_free_workers.Swap(worker, m_busy_workers);

    m_busy_workers.Back()->GatherVespene(refinery_);
}

bool Hub::AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_) {
    if (order_->assignee) {
        if (m_assignedBuildings.find(order_->assignee->tag) == m_assignedBuildings.end()
            && order_->assignee->orders.empty()) {
            m_assignedBuildings.insert(order_->assignee->tag);
            return true;
        }
    } else {
        for (const auto& unit : gAPI->observer().GetUnits(IsIdleUnit(building_), sc2::Unit::Alliance::Self)) {
            if (m_assignedBuildings.find(unit->tag) == m_assignedBuildings.end() && unit->orders.empty()) {
                m_assignedBuildings.insert(unit->tag);
                order_->assignee = unit;
                return true;
            }
        }
    }
    return false;
}

bool Hub::AssignBuildingProduction(Order *order_, sc2::UNIT_TYPEID building_, sc2::UNIT_TYPEID addon_requirement_) {
    if (order_->assignee) {
        return AssignBuildingProduction(order_);
    }

    for (const auto& unit : gAPI->observer().GetUnits(
            MultiFilter(MultiFilter::Selector::And, {IsIdleUnit(building_), HasAddon(addon_requirement_)}),
            sc2::Unit::Alliance::Self)) {
        if (m_assignedBuildings.find(unit->tag) == m_assignedBuildings.end() && unit->orders.empty()) {
            m_assignedBuildings.insert(unit->tag);
            order_->assignee = unit;
            return true;
        }
    }
    return false;
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

std::unique_ptr<Hub> gHub;
