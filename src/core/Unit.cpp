#include "Unit.h"

#include "UnitData.h"
#include "API.h"

#include "plugins/micro/MicroPlugin.h"

Unit::Unit(std::shared_ptr<UnitData> data) : m_data(std::move(data)) { }

Unit::Unit(const sc2::Unit* unit) {
    this->m_data = gAPI->WrapUnit(unit).m_data;
}

Unit::operator const sc2::Unit&() const {
    return *m_data->unit;
}

Unit::operator const sc2::Unit*() const {
    return m_data->unit;
}

const sc2::Unit* Unit::operator->() const {
    return m_data->unit;
}

const sc2::Unit& Unit::operator*() const {
    return *m_data->unit;
}

void Unit::InstallMicro() {
    if (!m_data->micro)
        m_data->micro = MicroPlugin::MakePlugin(*this);
}

MicroPlugin* Unit::Micro() const {
    return m_data->micro.get();
}
