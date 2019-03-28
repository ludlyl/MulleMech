#include "ArmyBuilding.h"

#include "../BuildingPlacer.h"
#include "../Hub.h"
#include "../Historican.h"
#include "core/API.h"
#include "core/Timer.h"

bool ArmyBuilding::Build(Order* order_) {
    Timer timer;
    timer.Start();
    auto pos = BuildingPlacer::FindPlaceInFrontOfCC(*order_, gAPI->observer().StartingLocation());
    auto ms = timer.Finish();
    gHistory.debug() << "Placing army producing structure took " << ms << " ms" << std::endl;
    if (pos.has_value())
        return gHub->AssignBuildTask(order_, pos.value());
    return false;
}
