#include "Order.h"
#include "Helpers.h"
#include "core/Unit.h"

Order::Order(const sc2::UnitTypeData &data_, Unit* assignee_) :
        name(data_.name),
        mineral_cost(data_.mineral_cost),
        vespene_cost(data_.vespene_cost),
        food_required(data_.food_required),
        structure_tech_requirements(GetAllStructureTechRequirements(data_)),
        unit_type_id(data_.unit_type_id),
        ability_id(data_.ability_id),
        tech_alias(data_.tech_alias),
        assignee(assignee_) { }

Order::Order(const sc2::UpgradeData &data_) :
        name(data_.name),
        mineral_cost(static_cast<int>(data_.mineral_cost)),
        vespene_cost(static_cast<int>(data_.vespene_cost)),
        food_required(0.0f),
        structure_tech_requirements(GetAllStructureTechRequirements(data_.ability_id)),
        upgrade_tech_requirement(GetUpgradeTechRequirement(data_.ability_id)),
        unit_type_id(sc2::UNIT_TYPEID::INVALID),
        // With the current API (last checked 2019-04-25) some upgrades return the wrong ability id
        // It's possible to manually fix this
        // (e.g. command center has mapped upgrade id to ability id manually, see TechTree.cpp in that project)
        ability_id(data_.ability_id) {
}
