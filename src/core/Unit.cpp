#include "Unit.h"

#include "API.h"

#include "plugins/micro/MicroPlugin.h"

Unit::Unit(const sc2::Unit& unit) : sc2::Unit(unit) { }

Unit::operator const sc2::Unit*() const {
    return static_cast<const sc2::Unit*>(this);
}

void Unit::InstallMicro() {
    if (!m_micro)
        m_micro = MicroPlugin::MakePlugin(this);
}

MicroPlugin* Unit::Micro() const {
    return m_micro.get();
}

std::unique_ptr<Unit> Unit::Make(const sc2::Unit& unit) {
    return std::make_unique<Unit>(unit);
}

void Unit::UpdateAPIData(const sc2::Unit& unit) {
    sc2::Unit::operator=(unit);
}
