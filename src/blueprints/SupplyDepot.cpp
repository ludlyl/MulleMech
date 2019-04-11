#include "SupplyDepot.h"
#include "BuildingPlacer.h"
#include "Hub.h"
#include "Historican.h"
#include "core/API.h"
#include "core/Timer.h"

bool bp::SupplyDepot::CanBeBuilt(const Order *order_) {
    Timer timer;
    timer.Start();
    auto pos = BuildingPlacer::CalculateFreePlaceBehindMinerals(*order_, gAPI->observer().StartingLocation());
    auto ms = timer.Finish();
    gHistory.debug() << "Trying to calculate placement for supply depot took " << ms << " ms" << std::endl;
    if (pos.has_value())
        return gHub->GetClosestFreeWorker(pos.value()) != nullptr;
    return false;
}

bool bp::SupplyDepot::Build(Order* order_) {
    Timer timer;
    timer.Start();
    auto pos = BuildingPlacer::CalculateFreePlaceBehindMinerals(*order_, gAPI->observer().StartingLocation());
    auto ms = timer.Finish();
    gHistory.debug() << "Trying to calculate placement for supply depot took " << ms << " ms" << std::endl;
    if (pos.has_value())
        return gHub->AssignBuildTask(order_, pos.value());
    return false;
}
