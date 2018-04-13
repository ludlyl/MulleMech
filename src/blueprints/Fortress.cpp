// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "../API.h"
#include "Fortress.h"
#include "../Helpers.h"

Fortress::Fortress(): Blueprint(false) {
}

bool Fortress::Build(Order* order_) {
    sc2::Units command_centers = gAPI->observer().GetUnits(IsFreeCommandCenter());
    if (command_centers.empty())
        return false;

    order_->assignee = command_centers.front();

    gAPI->action().Command(*order_);

    return true;
}

bool Fortress::TechRequirementsMet(const Order&) {
    // NOTE(alkurbatov): Unfortunally SC2 API returns wrong
    // tech_requirement for a planetary fortress so we use a workaround.
    // See https://github.com/Blizzard/s2client-api/issues/191

    return gAPI->observer().CountUnitType(sc2::UNIT_TYPEID::TERRAN_ENGINEERINGBAY) > 0;
}