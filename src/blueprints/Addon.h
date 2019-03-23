#pragma once

#include "Blueprint.h"

namespace bp {

struct Addon : Blueprint {
    // Could add a constructor that takes a "who_builds" (as the Unit struct does)
    bool Build(Order *order_) final;

    static sc2::UNIT_TYPEID GetParentStructureFromAbilityId(sc2::ABILITY_ID abilityId);
};

}
