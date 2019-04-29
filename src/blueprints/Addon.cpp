#include "Addon.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Historican.h"
#include "Hub.h"

bool bp::Addon::CanBeBuilt(const Order* order_) {
    return GetValidAssignee(order_) != nullptr;
}

bool bp::Addon::Build(Order *order_) {
    // TODO: If there isn't any space for the add-on (on any parent building), lift and re-place the building

    Unit* assignee = GetValidAssignee(order_);
    if (gHub->AssignBuildingProduction(order_, assignee)) {
        gAPI->action().Build(*order_);
        return true;
    }

    return false;
}

sc2::UNIT_TYPEID bp::Addon::GetParentStructureFromAbilityId(sc2::ABILITY_ID abilityId) {
    switch(abilityId) {
        case sc2::ABILITY_ID::BUILD_TECHLAB_BARRACKS:
        case sc2::ABILITY_ID ::BUILD_REACTOR_BARRACKS:
            return sc2::UNIT_TYPEID::TERRAN_BARRACKS;
        case sc2::ABILITY_ID::BUILD_TECHLAB_FACTORY:
        case sc2::ABILITY_ID ::BUILD_REACTOR_FACTORY:
            return sc2::UNIT_TYPEID::TERRAN_FACTORY;
        case sc2::ABILITY_ID::BUILD_TECHLAB_STARPORT:
        case sc2::ABILITY_ID ::BUILD_REACTOR_STARPORT:
            return sc2::UNIT_TYPEID::TERRAN_STARPORT;
        default:
            return sc2::UNIT_TYPEID::INVALID;
    }
}

Unit* bp::Addon::GetValidAssignee(const Order* order_) {
    Unit* assignee = nullptr;
    // As doing "CanBePlaced" is bugged on add-ons, we use another 2x2 building to check it instead
    Order supplyDepotOrder(gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT));
    auto buildingType = GetParentStructureFromAbilityId(order_->ability_id);

    if (!order_->assignee) {
        // Get all idle parent buildings that doesn't already have an add-on
        auto parent_buildings = gAPI->observer().GetUnits(
                MultiFilter(MultiFilter::Selector::And, {IsIdleUnit(buildingType, false), HasAddon(sc2::UNIT_TYPEID::INVALID)}),
                sc2::Unit::Alliance::Self);

        for (auto& building : parent_buildings) {
            // Check if the addon can be placed
            if (gAPI->query().CanBePlaced(supplyDepotOrder, GetTerranAddonPosition(gAPI->observer().GetUnit(building->tag)))) {
                assignee = building;
                break;
            }
        }

        // Return nullptr if no parent building that fulfilled the requirements was found
        if (!assignee) {
            return nullptr;
        }
    } else {
        // Should there be more checks here (when the assignee/parent structure was already supplied) to see if the order will work?
        // (e.g. check if it's the right kind of parent structure, if the parent structure doesn't already have an add-on etc.)
        // Or is it preferred to let the function return true even if the action will fail?

        // Check if the addon can be placed
        if (!gAPI->query().CanBePlaced(supplyDepotOrder, GetTerranAddonPosition(order_->assignee))) {
            return nullptr;
        }
    }

    return assignee;
}
