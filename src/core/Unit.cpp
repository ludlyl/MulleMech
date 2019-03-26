#include "Unit.h"

#include "API.h"
#include "Helpers.h"
#include "objects/Worker.h"

#include "plugins/micro/MicroPlugin.h"

Unit::Unit(const sc2::Unit& unit) : sc2::Unit(unit) { }

Unit::operator const sc2::Unit*() const {
    return static_cast<const sc2::Unit*>(this);
}

bool Unit::operator==(const Unit& other) const {
    return tag == other.tag;
}

void Unit::InstallMicro() {
    if (!m_micro)
        m_micro = MicroPlugin::MakePlugin(this);
}

MicroPlugin* Unit::Micro() const {
    return m_micro.get();
}

std::unique_ptr<Unit> Unit::Make(const sc2::Unit& unit) {
    switch (unit.unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
            return std::make_unique<Worker>(unit);

        default:
            return std::make_unique<Unit>(unit);
    }
}

void Unit::UpdateAPIData(const sc2::Unit& unit) {
    assert(tag == unit.tag);
    sc2::Unit::operator=(unit);
}

Worker* Unit::AsWorker() {
    if (auto worker = dynamic_cast<Worker*>(this)) {
        return worker;
    } else {
        if (IsWorker()(*this))
            throw std::runtime_error("Unit with worker typeid must be of type Worker");
    }
    return nullptr;
}

sc2::UnitTypeData Unit::GetTypeData() const {
    return gAPI->observer().GetUnitTypeData(this->unit_type);
}
