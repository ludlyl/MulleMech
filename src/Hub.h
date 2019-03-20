// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Unit.h"
#include "core/Map.h"
#include "objects/Geyser.h"
#include "objects/Worker.h"

#include <sc2api/sc2_gametypes.h>
#include <sc2api/sc2_unit.h>

#include <list>
#include <memory>
#include <unordered_set>
#include <optional>

template <typename T>
struct Cache {
    bool Empty() const;

    uint64_t Count() const;

    void Add(const T& obj_);

    T& Back();

    void PopBack();

    bool IsCached(const T& obj_) const;

    bool Swap(const T& obj_, Cache<T>& dst_);

    bool Remove(const T& obj_);

    T* GetClosestTo(const sc2::Point2D& location_);

 private:
    std::list<T> m_objects;
};

template <typename T>
bool Cache<T>::Empty() const {
    return m_objects.empty();
}

template <typename T>
uint64_t Cache<T>::Count() const {
    return m_objects.size();
}

template <typename T>
void Cache<T>::Add(const T& obj_) {
    m_objects.push_back(obj_);
}

template <typename T>
T& Cache<T>::Back() {
    return m_objects.back();
}

template <typename T>
void Cache<T>::PopBack() {
    m_objects.pop_back();
}

template <typename T>
bool Cache<T>::IsCached(const T& obj_) const {
    auto it = std::find(m_objects.begin(), m_objects.end(), obj_);

    return m_objects.end() != it;
}

template <typename T>
bool Cache<T>::Swap(const T& obj_, Cache<T>& dst_) {
    auto it = std::find(m_objects.begin(), m_objects.end(), obj_);

    if (m_objects.end() == it)
        return false;

    dst_.Add(*it);
    m_objects.erase(it);
    return true;
}

template <typename T>
bool Cache<T>::Remove(const T& obj_) {
    auto it = std::find(m_objects.begin(), m_objects.end(), obj_);
    if (m_objects.end() == it)
        return false;

    m_objects.erase(it);
    return true;
}

template <typename T>
T* Cache<T>::GetClosestTo(const sc2::Point2D& location_) {
    auto closest_worker = m_objects.end();
    float distance = std::numeric_limits<float>::max();

    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        float d = sc2::DistanceSquared2D(it->GetPos(), location_);

        if (d >= distance)
            continue;

        distance = d;
        closest_worker = it;
    }

    if (closest_worker == m_objects.end())
        return nullptr;

    return &(*closest_worker);
}

struct Construction {
    Construction(const Unit& building_, const Unit& scv_);
    std::optional<Unit> GetBuilding() const;
    std::optional<Unit> GetScv() const;
    sc2::Tag building;
    sc2::Tag scv;
};

struct Hub {
    Hub(sc2::Race current_race_, const Expansions& expansions_);

    void OnStep();

    void OnUnitCreated(Unit unit_);

    void OnUnitDestroyed(Unit unit_);

    void OnUnitIdle(Unit unit_);

    void OnBuildingConstructionComplete(Unit building_);

    bool IsOccupied(const Unit& unit_) const;

    bool IsTargetOccupied(const sc2::UnitOrder& order_) const;

    void ClaimObject(const Unit& unit_);

    sc2::Race GetCurrentRace() const;

    Worker* GetClosestFreeWorker(const sc2::Point2D& location_);

    sc2::UNIT_TYPEID GetCurrentWorkerType() const;

    bool AssignRefineryConstruction(Order* order_, const Unit& geyser_);

    bool AssignBuildTask(Order* order_, const sc2::Point2D& point_);

    void AssignVespeneHarvester(const Unit& refinery_);

    // Find first free building to produce Units/Upgrades/Addons/Mutations from/on
    bool AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_ = sc2::UNIT_TYPEID::INVALID);

    // If INVALID is sent in as a addon_requirement (and no assignee is provided) the order is assigned to a unit with no add-on
    bool AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_, sc2::UNIT_TYPEID addon_requirement_);

    const Expansions& GetExpansions() const;

    std::shared_ptr<Expansion> GetClosestExpansion(const sc2::Point2D& location_) const;

    std::vector<Construction>& GetConstructions() { return m_constructions; }

 private:
    sc2::Race m_current_race;
    Expansions m_expansions;
    sc2::UNIT_TYPEID m_current_worker_type;

    Cache<Geyser> m_captured_geysers;

    Cache<Worker> m_busy_workers;
    Cache<Worker> m_free_workers;

    std::unordered_set<sc2::Tag> m_assignedBuildings;
    std::vector<Construction> m_constructions;
};

extern std::unique_ptr<Hub> gHub;
