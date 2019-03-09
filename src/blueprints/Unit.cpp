// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Unit.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Hub.h"

Unit::Unit(sc2::UNIT_TYPEID who_builds_): m_who_builds(who_builds_) {
}

// TODO: Fix for add-ons
bool Unit::Build(Order* order_) {
    if (!gHub->AssignBuildingProduction(m_who_builds, order_))
        return false;

    gAPI->action().Build(*order_);

    return true;
}
