#include "Unit.h"
#include "API.h"
#include "Helpers.h"
#include "objects/Worker.h"
#include "plugins/micro/MicroPlugin.h"

Unit::Unit(const sc2::Unit& unit_) : sc2::Unit(unit_), IsInVision(true) {
    if (unit_.alliance == Unit::Alliance::Self) {
        m_micro = MicroPlugin::MakePlugin(this);
    }
}

bool Unit::operator==(const Unit& other_) const {
    return tag == other_.tag;
}

MicroPlugin* Unit::Micro() {
    return m_micro.get();
}

std::unique_ptr<Unit> Unit::Make(const sc2::Unit& unit_) {
    switch (unit_.unit_type.ToType()) {
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
            return std::make_unique<Worker>(unit_);

        default:
            return std::make_unique<Unit>(unit_);
    }
}

void Unit::UpdateAPIData(const sc2::Unit& unit) {
    assert(tag == unit.tag);
    sc2::Unit::operator=(unit);
}

bool Unit::IsIdle() const {
    return orders.empty() && !m_order_queued_in_current_step;
}

int Unit::NumberOfOrders() const {
    if (m_order_queued_in_current_step) {
        return static_cast<int>(orders.size() + 1);
    } else {
        return static_cast<int>(orders.size());
    }
}

const std::vector<sc2::UnitOrder>& Unit::GetPreviousStepOrders() const {
    return orders;
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

Unit * Unit::GetAttachedAddon() const {
    return gAPI->observer().GetUnit(this->add_on_tag);
}

Unit::Attackable Unit::CanAttack(const Unit* other_) const {
    auto our_data = gAPI->observer().GetUnitTypeData(unit_type);

    bool has_wep_type = false;
    for (auto& weapon : our_data.weapons) {
        if (sc2::Distance3D(pos, other_->pos) < weapon.range)
            continue;

        if (weapon.type == sc2::Weapon::TargetType::Any)
            has_wep_type = true;
        else if (weapon.type == sc2::Weapon::TargetType::Ground && !other_->is_flying)
            has_wep_type = true;
        else if (weapon.type == sc2::Weapon::TargetType::Air && other_->is_flying)
            has_wep_type = true;

        if (has_wep_type)
            break;
    }

    if (!has_wep_type)
        return Attackable::no;

    if (other_->cloak == sc2::Unit::Cloaked)
        return Attackable::need_scan;

    return Attackable::yes;
}

bool Unit::CanAttackFlying() const {
    auto our_data = gAPI->observer().GetUnitTypeData(unit_type);

    for (auto& weapon : our_data.weapons) {
        if (weapon.type == sc2::Weapon::TargetType::Any || weapon.type == sc2::Weapon::TargetType::Air)
            return true;
    }

    return false;
}

int Unit::GetValue() const {
    auto type_data = GetTypeData();
    return type_data.mineral_cost + static_cast<int>(type_data.vespene_cost * VespeneCostMod);
}
