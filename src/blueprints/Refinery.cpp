#include "Refinery.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "BuildingPlacer.h"

bool bp::Refinery::CanBeBuilt(const Order*) {
    // This is needed as Expansion::geysers are not updated for some reason
    auto visible_geysers = gAPI->observer().GetUnits(IsGeyser(), sc2::Unit::Alliance::Neutral);

    if (FreeWorkerExists(true)) {
        for (auto& expansion : gHub->GetOurExpansions()) {
            for (auto& geyser_position : expansion->geysers_positions) {
                auto geyser = visible_geysers.GetClosestUnit(geyser_position);
                // Note: only geysers that are in vision will report a vespene content > 0
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

    // This is needed as Expansion::geysers are not updated for some reason
    auto visible_geysers = gAPI->observer().GetUnits(IsVisibleUndepletedGeyser(), sc2::Unit::Alliance::Neutral);

    for (auto& expansion : gHub->GetOurExpansions()) {
        for (auto& geyser_position : expansion->geysers_positions) {
            auto geyser = visible_geysers.GetClosestUnit(geyser_position);
            // Note: only geysers that are in vision will report a vespene content > 0
            if (geyser->vespene_contents > 0 && gBuildingPlacer->ReserveGeyser(geyser)) {
                Worker* worker = GetClosestFreeWorker(geyser->pos);
                // If no unemployed or mineral workers exists, try getting a free gas worker
                // (this is a pretty inefficient way to solve this)
                if (!worker) {
                    worker = GetClosestFreeWorker(geyser->pos, true);
                }
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
