#include "Order.h"
#include "Helpers.h"

Order::Order(const sc2::UnitTypeData &data_, const sc2::Unit *assignee_) :
        name(data_.name),
        mineral_cost(data_.mineral_cost),
        vespene_cost(data_.vespene_cost),
        food_required(data_.food_required),
        tech_requirements(GetAllTechRequirements(data_)),
        unit_type_id(data_.unit_type_id),
        ability_id(data_.ability_id),
        tech_alias(data_.tech_alias) {
    if (assignee_)
        assignee = assignee_->tag;
}

Order::Order(const sc2::UpgradeData &data_) :
        name(data_.name),
        mineral_cost(static_cast<int>(data_.mineral_cost)),
        vespene_cost(static_cast<int>(data_.vespene_cost)),
        food_required(0.0f),
        tech_requirements(GetAllTechRequirements(data_.ability_id)),
        unit_type_id(sc2::UNIT_TYPEID::INVALID),
        ability_id(data_.ability_id) {
}
