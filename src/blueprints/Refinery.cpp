#include "Refinery.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "BuildingPlacer.h"

bool bp::Refinery::CanBeBuilt(const Order*) {
    if (FreeWorkerExists()) {
        auto geysers = gAPI->observer().GetUnits(IsVisibleUndepletedGeyser());
        for (const auto& geyser : geysers) {
            if (gBuildingPlacer->IsGeyserUnoccupied(geyser))
                return true;
        }
    }
    return false;
}

bool bp::Refinery::Build(Order* order_) {
    // Should "FreeWorkerExists" be checked here too? CanBeBuilt is always called before this so feels necessary,
    // but it's really bad if ReserveBuildingSpace is called, succeeds and no free worker is found

    auto geysers = gAPI->observer().GetUnits(IsVisibleUndepletedGeyser());
    const auto& starting = gAPI->observer().StartingLocation();
    std::sort(geysers.begin(), geysers.end(), [&starting](auto& g1, auto& g2) {
        return sc2::Distance2D(starting, g1->pos) < sc2::Distance2D(starting, g2->pos);
    });


    for (const auto& geyser : geysers) {
        if (gBuildingPlacer->ReserveGeyser(geyser)) {
            Worker* worker = GetClosestFreeWorker(geyser->pos);
            if (worker) {
                worker->BuildRefinery(order_, geyser);
                return true;
            } else {
                assert(false && "Refinery space reserved but no free worker was found!");
            }
        }
    }
    return false;
}
