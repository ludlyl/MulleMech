// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Refinery.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

bool bp::Refinery::Build(Order* order_) {
    auto geysers = gAPI->observer().GetUnits(IsFreeGeyser(),
        sc2::Unit::Alliance::Neutral);

    auto geyser = geysers.GetClosestUnit(gAPI->observer().StartingLocation());
    if (!geyser)
        return false;

    return gHub->AssignRefineryConstruction(order_, geyser);
}
