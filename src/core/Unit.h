#pragma once

#include "plugins/micro/MicroPlugin.h"
#include <sc2api/sc2_unit.h>
#include <memory>

class Worker;

namespace API { struct Interface; }
namespace sc2 { struct UnitTypeData; }

class Unit : public sc2::Unit {
public:
    static std::unique_ptr<Unit> Make(const sc2::Unit& unit);
    Unit(const sc2::Unit& unit);
    virtual ~Unit() = default;
    void UpdateAPIData(const sc2::Unit& unit);

    // Implicit conversion: Unit -> const sc2::Unit*
    operator const sc2::Unit*() const;

    bool operator==(const Unit& other) const;

    // Micro plugin for this unit
    void InstallMicro();
    MicroPlugin* Micro() const;

    Worker* AsWorker();

    sc2::UnitTypeData GetTypeData() const;

private:
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
