#include "Unit.h"
#include "API.h"
#include "Helpers.h"
#include "objects/Worker.h"
#include "plugins/micro/MicroPlugin.h"

Unit::Unit(const sc2::Unit& unit) : sc2::Unit(unit), IsInVision(true) {
    if (unit.alliance == Unit::Alliance::Self) {
        m_micro = MicroPlugin::MakePlugin(this);
    }
}

bool Unit::operator==(const Unit& other) const {
    return tag == other.tag;
}

MicroPlugin* Unit::Micro() {
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
    return const_cast<Worker*>(const_cast<const Unit*>(this)->AsWorker());
}

const Worker* Unit::AsWorker() const {
    if (auto worker = dynamic_cast<const Worker*>(this)) {
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
