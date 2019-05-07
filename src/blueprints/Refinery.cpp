#include "Refinery.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "BuildingPlacer.h"

bool bp::Refinery::CanBeBuilt(const Order*) {
    // This is needed as Expansion::geysers are not updated for some reason
    auto visible_geysers = gAPI->observer().GetUnits(IsVisibleUndepletedGeyser(), sc2::Unit::Alliance::Neutral);

    if (FreeWorkerExists()) {
        for (auto& expansion : gHub->GetOurExpansions()) {
            for (auto& geyser : expansion->geysers) {
                auto updated_geyser = visible_geysers.GetClosestUnit(geyser->pos);
                // We do not need to check the vespene content as visible_geysers only contains undepleted geysers
                if (geyser->pos.x == updated_geyser->pos.x && geyser->pos.y == updated_geyser->pos.y
                    && gBuildingPlacer->IsGeyserUnoccupied(updated_geyser)) {
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

    // This is needed as Expansion::geysers are not updated for some reason
    auto visible_geysers = gAPI->observer().GetUnits(IsVisibleUndepletedGeyser(), sc2::Unit::Alliance::Neutral);

    for (auto& expansion : gHub->GetOurExpansions()) {
        for (auto& geyser : expansion->geysers) {
            auto updated_geyser = visible_geysers.GetClosestUnit(geyser->pos);
            // We do not need to check the vespene content as visible_geysers only contains undepleted geysers
            if (geyser->pos.x == updated_geyser->pos.x && geyser->pos.y == updated_geyser->pos.y
                && gBuildingPlacer->ReserveGeyser(updated_geyser)) {
                Worker* worker = GetClosestFreeWorker(geyser->pos);
                if (worker) {
                    worker->BuildRefinery(order_, updated_geyser);
                    worker->construction = std::make_unique<Construction>(updated_geyser->pos, order_->unit_type_id);
                    return true;
                } else {
                    assert(false && "Refinery space reserved but no free worker was found!");
                }
            }
        }
    }
    return false;
}
