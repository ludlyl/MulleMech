#pragma once

#include <sc2api/sc2_unit.h>
#include <memory>

class MicroPlugin;

namespace API { struct Interface; }

class Unit : public sc2::Unit {
public:
    static std::unique_ptr<Unit> Make(const sc2::Unit& unit);
    Unit(const sc2::Unit& unit);
    void UpdateAPIData(const sc2::Unit& unit);

    // Implicit conversion: Unit -> const sc2::Unit*
    operator const sc2::Unit*() const;

    // Micro plugin for this unit
    void InstallMicro();
    MicroPlugin* Micro() const;

private:
    std::unique_ptr<MicroPlugin> m_micro;
};
