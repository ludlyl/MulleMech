#pragma once

#include "Blueprint.h"

namespace bp {

struct Addon : Blueprint {
    // Could add a constructor that takes a "who_builds" (as the Unit struct does)
    bool CanBeBuilt(const Order* order_) final;

    bool Build(Order *order_) final;

    static sc2::UNIT_TYPEID GetParentStructureFromAbilityId(sc2::ABILITY_ID abilityId);

private:
    // Finds (and returns) a free building the addon can be built on, or if an assignee is already set,
    // checks if that assignee is free & it's possible to build an addon on that building
    // The point of this function is basically just to avoid code duplication between CanBeBuilt and Build
    Unit* GetValidAssignee(const Order* order_);
};

}
