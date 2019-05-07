#include "Refinery.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "BuildingPlacer.h"

bool bp::Refinery::CanBeBuilt(const Order*) {
    if (FreeWorkerExists()) {
        for (auto& expansion : gHub->GetOurExpansions()) {
            for (auto& geyser : expansion->geysers) {
                if (geyser->vespene_contents > 0 && gBuildingPlacer->IsGeyserUnoccupied(geyser)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool bp::Refinery::Build(Order* order_) {
    // Should "FreeWorkerExists" be checked here too? CanBeBuilt is always called before this so feels necessary,
    // but it's really bad if ReserveBuildingSpace is called, succeeds and no free worker is found

    for (auto& expansion : gHub->GetOurExpansions()) {
        for (auto& geyser : expansion->geysers) {
            if (geyser->vespene_contents > 0 && gBuildingPlacer->ReserveGeyser(geyser)) {
                Worker* worker = GetClosestFreeWorker(geyser->pos);
                if (worker) {
                    worker->BuildRefinery(order_, geyser);
                    worker->construction = std::make_unique<Construction>(geyser->pos, order_->unit_type_id);
                    return true;
                } else {
                    assert(false && "Refinery space reserved but no free worker was found!");
                }
            }
        }
    }
    return false;
}
