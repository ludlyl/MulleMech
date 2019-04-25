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
    // Should "FreeWorkerExists" be checked here too? CanBeBuilt is always called before this so feels necessary,
    // but it's really bad if ReserveBuildingSpace is called, succeeds and no free worker is found

    auto optional_pos = gBuildingPlacer->ReserveBuildingSpace(*order_,
                                                              IsBuildingWithSupportForAddon()(order_->unit_type_id));
    if (optional_pos.has_value()) {
        Worker* worker = GetClosestFreeWorker(optional_pos.value());
        if (worker) {
            worker->Build(order_, optional_pos.value());
            return true;
        } else {
            assert(false && "Building space reserved but no free worker was found!");
        }
    }
    return false;
}
