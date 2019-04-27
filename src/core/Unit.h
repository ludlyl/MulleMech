#pragma once

#include "plugins/micro/MicroPlugin.h"

#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_data.h>

#include <memory>

class Worker;

namespace API {
    struct Action;
    struct Interface;
}

class Unit : public sc2::Unit {
    friend API::Action; // Needed to set m_order_queued_in_current_step
    friend API::Interface;

public:
    // TODO: Make "Make" function and constructor private (and same for worker) so only API can create new Unit objects
    static std::unique_ptr<Unit> Make(const sc2::Unit& unit);
    Unit(const sc2::Unit& unit);
    Unit(const Unit&) = delete;
    virtual ~Unit() = default;
    bool operator==(const Unit& other) const;

    // I.e. is sc2::Unit::orders empty and m_order_queued_in_current_step = false
    bool IsIdle() const;

    // Might want a better name for this (OrdersSize?)
    int NumberOfOrders() const;

    const std::vector<sc2::UnitOrder>& GetPreviousStepOrders() const;

    // Micro plugin for this unit
    MicroPlugin* Micro();

    Worker* AsWorker();
    const Worker* AsWorker() const;

    sc2::UnitTypeData GetTypeData() const;

    bool IsInVision; // False if unit is no longer visible to us (either dead or in fog of war)

    enum class BuildingRepairPhase {
        not_repairing,
        repairing
    };
    BuildingRepairPhase m_repairPhase = BuildingRepairPhase::not_repairing;

private:
    // Makes sc2::Unit::orders private, this isn't a very pretty solution but as sc2::Unit::orders
    // should never be used outside of this class it might be good to pick up on some bugs
    using sc2::Unit::orders;

    // Updates the API data (sc2::Unit) too
    void UpdateAPIData(const sc2::Unit& unit);

    // This is set to true if an order (attack, move, stop etc.) has been given to the unit.
    // This variable should be reset at the start of every step (by calling the Update function).
    // (An alternative to having an update function would be to save the step number instead)
    bool m_order_queued_in_current_step = false;

    std::unique_ptr<MicroPlugin> m_micro;
};

// Allow hashmap usage
namespace std {

template<> struct hash<Unit> {
    typedef Unit argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const noexcept {
        return std::hash<sc2::Tag>{}(s.tag);
    }
};

}
