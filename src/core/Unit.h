#pragma once

#include "plugins/micro/MicroPlugin.h"

#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_data.h>

#include <memory>

class Worker;

class Unit : public sc2::Unit {
public:
    static std::unique_ptr<Unit> Make(const sc2::Unit& unit);
    Unit(const sc2::Unit& unit);
    Unit(const Unit&) = delete;
    virtual ~Unit() = default;
    bool operator==(const Unit& other) const;

    void UpdateAPIData(const sc2::Unit& unit);

    // Micro plugin for this unit
    MicroPlugin* Micro();

    Worker* AsWorker();
    const Worker* AsWorker() const;

    sc2::UnitTypeData GetTypeData() const;

    bool IsInVision; // False if unit is no longer visible to us (either dead or in fog of war)

private:
    // Makes sc2::Unit::orders private, this isn't a very pretty solution but as sc2::Unit::orders
    // should never be used outside of this class it might be good to pick up on some bugs

    // TODO: Fix orders
//    using sc2::Unit::orders;
//
//    UnitOrder

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
