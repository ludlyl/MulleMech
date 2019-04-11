#include "ProductionBuilding.h"
#include "core/API.h"
#include "core/Timer.h"
#include "BuildingPlacer.h"
#include "Hub.h"
#include "Historican.h"

bool bp::ProductionBuilding::CanBeBuilt(const Order* order_) {
    Timer timer;
    timer.Start();
    auto pos = BuildingPlacer::CalculateFreePlaceInFrontOfTownHall(*order_, gAPI->observer().StartingLocation(), true);
    auto ms = timer.Finish();
    gHistory.debug() << "Trying to calculate placement for production building took " << ms << " ms" << std::endl;
    if (pos.has_value())
        return gHub->GetClosestFreeWorker(pos.value()) != nullptr;
    return false;
}

bool bp::ProductionBuilding::Build(Order* order_) {
    Timer timer;
    timer.Start();
    auto pos = BuildingPlacer::CalculateFreePlaceInFrontOfTownHall(*order_, gAPI->observer().StartingLocation(), true);
    auto ms = timer.Finish();
    gHistory.debug() << "Trying to calculate placement for production building took " << ms << " ms" << std::endl;
    if (pos.has_value())
        return gHub->AssignBuildTask(order_, pos.value());
    return false;
}
