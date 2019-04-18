// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Unit.h"
#include "core/Map.h"
#include "objects/Worker.h"

#include <sc2api/sc2_gametypes.h>
#include <sc2api/sc2_unit.h>

#include <functional>
#include <list>
#include <memory>
#include <optional>

template <typename T>
struct Cache {
    bool Empty() const;

    uint64_t Count() const;

    void Add(T* obj_);

    T* Back();

    void PopBack();

    bool Contains(const T* obj_) const;

    bool Swap(const T* obj_, Cache<T>& dst_);

    bool Remove(const T* obj_);

    // When locations are reserved (i.e., Geysers)
    bool IsOccupied(const T* obj_) const;
    bool RemoveOccupied(const T* obj_);

    T* GetClosestTo(const sc2::Point2D& location_);

 private:
    std::list<T*> m_objects;
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
void Cache<T>::Add(T* obj_) {
    m_objects.push_back(obj_);
}

template <typename T>
T* Cache<T>::Back() {
    return m_objects.back();
}

template <typename T>
void Cache<T>::PopBack() {
    m_objects.pop_back();
}

template <typename T>
bool Cache<T>::Contains(const T* obj_) const {
    auto it = std::find(m_objects.begin(), m_objects.end(), obj_);

    return m_objects.end() != it;
}

template <typename T>
bool Cache<T>::Swap(const T* obj_, Cache<T>& dst_) {
    auto it = std::find(m_objects.begin(), m_objects.end(), obj_);

    if (m_objects.end() == it)
        return false;

    dst_.Add(*it);
    m_objects.erase(it);
    return true;
}

template <typename T>
bool Cache<T>::Remove(const T* obj_) {
    auto it = std::find(m_objects.begin(), m_objects.end(), obj_);
    if (m_objects.end() == it)
        return false;

    m_objects.erase(it);
    return true;
}

template <typename T>
bool Cache<T>::IsOccupied(const T* obj_) const {
    for (auto& obj : m_objects) {
        if (obj == obj_ ||
            (obj_->pos.x == obj->pos.x && obj_->pos.y == obj->pos.y))
            return true;

    }
    return false;
}

template <typename T>
bool Cache<T>::RemoveOccupied(const T* obj_) {
    for (auto itr = m_objects.begin(); itr != m_objects.end(); ++itr) {
        if (*itr == obj_ ||
            (obj_->pos.x == (*itr)->pos.x && obj_->pos.y == (*itr)->pos.y)) {
            m_objects.erase(itr);
            return true;
        }
    }

    return false;
}

template <typename T>
T* Cache<T>::GetClosestTo(const sc2::Point2D& location_) {
    auto closest_worker = m_objects.end();
    float distance = std::numeric_limits<float>::max();

    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        float d = sc2::DistanceSquared2D((*it)->pos, location_);

        if (d >= distance)
            continue;

        distance = d;
        closest_worker = it;
    }

    if (closest_worker == m_objects.end())
        return nullptr;

    return *closest_worker;
}

struct Construction {
    Construction(Unit* building_, Unit* scv_);
    Unit* GetBuilding() const;
    Unit* GetScv() const;
    Unit* building;
    Unit* scv;
};

struct Hub {
    Hub(sc2::Race current_race_, Expansions  expansions_);

    void OnUnitCreated(Unit* unit_);

    void OnUnitDestroyed(Unit* unit_);

    void OnUnitIdle(Unit* unit_);

    void OnBuildingConstructionComplete(Unit* building_);

    bool IsOccupied(const Unit* unit_) const;

    bool IsTargetOccupied(const sc2::UnitOrder& order_) const;

    void ClaimObject(Unit* unit_);

    sc2::Race GetCurrentRace() const;

    sc2::UNIT_TYPEID GetCurrentWorkerType() const;

    bool AssignRefineryConstruction(Order* order_, Unit* geyser_);

    bool AssignBuildTask(Order* order_, const sc2::Point2D& point_);

    void AssignVespeneHarvester(const Unit* refinery_);

    // Returns nullptr if no building to produce Units/Upgrades/Addons/Mutations from/on is available
    Unit* GetFreeBuildingProductionAssignee(const Order *order_, sc2::UNIT_TYPEID building_ = sc2::UNIT_TYPEID::INVALID);

    // If INVALID is sent in as a addon_requirement (and no assignee is provided) the order is assigned to a unit with no add-on
    Unit* GetFreeBuildingProductionAssignee(const Order *order_, sc2::UNIT_TYPEID building_,
                                            sc2::UNIT_TYPEID addon_requirement_);

    // This should always be used instead of manually setting the assignee for production
    bool AssignBuildingProduction(Order* order_, Unit* assignee);

    // Find first free building to produce Units/Upgrades/Addons/Mutations from/on
    bool AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_ = sc2::UNIT_TYPEID::INVALID);

    // If INVALID is sent in as a addon_requirement (and no assignee is provided) the order is assigned to a unit with no add-on
    bool AssignBuildingProduction(Order* order_, sc2::UNIT_TYPEID building_, sc2::UNIT_TYPEID addon_requirement_);

    const Expansions& GetExpansions() const;

    std::shared_ptr<Expansion> GetClosestExpansion(const sc2::Point2D& location_) const;

    std::vector<Construction>& GetConstructions() { return m_constructions; }

    // Returns a list of our expansions sorted with walking distance to starting location
    // index: 0=>main base, 1=>natural, etc
    Expansions GetOurExpansions() const;

 private:
    sc2::Race m_current_race;
    Expansions m_expansions;
    sc2::UNIT_TYPEID m_current_worker_type;

    Cache<Unit> m_captured_geysers;

    std::vector<Construction> m_constructions;
};

extern std::unique_ptr<Hub> gHub;
