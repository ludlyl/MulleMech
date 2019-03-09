// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Unit.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Hub.h"

Unit::Unit(sc2::UNIT_TYPEID who_builds_, std::optional<sc2::UNIT_TYPEID> required_addon_): m_who_builds(who_builds_), m_required_addon(required_addon_) {
}

bool Unit::Build(Order* order_) {
    if (!gHub->AssignBuildingProduction(order_, m_who_builds, m_required_addon))
        return false;

    gAPI->action().Build(*order_);

    return true;
}
