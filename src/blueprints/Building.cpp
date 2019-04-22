// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Building.h"
#include "Hub.h"
#include "core/Helpers.h"
#include "core/API.h"
#include "BuildingPlacer.h"

bool bp::Building::CanBeBuilt(const Order*) {
    return FreeWorkerExists();
}

bool bp::Building::Build(Order* order_) {
    auto optional_pos = gBuildingPlacer->ReserveBuildingSpace(*order_,
                                                              IsBuildingWithSupportForAddon()(order_->unit_type_id));
    if (optional_pos.has_value()) {
        return gHub->AssignBuildTask(order_, optional_pos.value());
    }
    return false;
}
