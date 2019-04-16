#include "SupplyDepot.h"
#include "BuildingPlacer.h"
#include "Hub.h"
#include "Historican.h"
#include "core/API.h"
#include "core/Timer.h"
#include "core/Helpers.h"

bool bp::SupplyDepot::CanBeBuilt(const Order* order_) {
    auto pos = FindBuildLocation(order_);
    if (pos.has_value())
        return GetClosestFreeWorker(pos.value()) != nullptr;
    return false;
}

bool bp::SupplyDepot::Build(Order* order_) {
    auto pos = FindBuildLocation(order_);
    if (pos.has_value())
        return gHub->AssignBuildTask(order_, pos.value());
    return false;
}

std::optional<sc2::Point3D> bp::SupplyDepot::FindBuildLocation(const Order* order_) const {
    Timer timer;
    timer.Start();

    std::optional<sc2::Point3D> pos = std::nullopt;
    auto expansions = gHub->GetOurExpansions();
    for (auto& expansion : expansions) {
        // TODO: Maybe we should cache which expansions are already full? If all are full this is gonna take forever
        pos = BuildingPlacer::CalculateFreePlaceBehindMinerals(*order_, expansion->town_hall_location);
        if (pos.has_value())
            break;
    }

    auto ms = timer.Finish();
    gHistory.debug() << "Trying to calculate placement for supply depot took " << ms << " ms" << std::endl;

    return pos;
}
