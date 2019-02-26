#include <core/API.h>
#include <core/Helpers.h>
#include <Historican.h>
#include "Addon.h"

bool Addon::Build(Order *order_) {
    // TODO: If there isn't any space for the add-on (on any parent building), lift and re-place the building

    // As doing "CanBePlaced" is bugged on add-ons, we use another 2x2 building to check it instead
    Order supplyDepotOrder(gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT));

    if (!order_->assignee) {
        auto parent_buildings = gAPI->observer().GetUnits(
                IsIdleUnit(GetParentStructureFromAbilityId(order_->ability_id)));
        // Return false if no such parent building for the add-on is found
        if (parent_buildings().empty())
            return false;

        // Build the add-on in the first parent building that doesn't already have an add-on
        for (auto &building : parent_buildings()) {
            if (building->add_on_tag == 0) {
                // Check if the addon can be placed
                if (gAPI->query().CanBePlaced(supplyDepotOrder, GetTerranAddonPosition(*(gAPI->observer().GetUnit(building->tag))))) {
                    order_->assignee = building->tag;
                    break;
                }
            }
        }

        // Return false if all parent buildings already had add-ons
        if (!order_->assignee) {
            return false;
        }
    } else {
        // Should there be more checks here (when the assignee/parent structure was already supplied) to see if the order will work?
        // (e.g. check if it's the right kind of parent structure, if the parent structure doesn't already have an add-on etc.)
        // Or is it preferred to let the function return true even if the action will fail?

        // Check if the addon can be placed
        if (!gAPI->query().CanBePlaced(supplyDepotOrder, GetTerranAddonPosition(*(gAPI->observer().GetUnit(order_->assignee))))) {
            return false;
        }
    }
    gAPI->action().Build(*order_);
    return true;
}

sc2::UNIT_TYPEID Addon::GetParentStructureFromAbilityId(sc2::ABILITY_ID abilityId) {
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
