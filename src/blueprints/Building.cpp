// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Building.h"
#include "Hub.h"
#include "core/API.h"

bool bp::Building::Build(Order* order_) {
    // Find place to build the structure
    sc2::Point3D base = gAPI->observer().StartingLocation();
    sc2::Point2D point;

    unsigned attempt = 0;
    do {
        point.x = base.x + sc2::GetRandomScalar() * 15.0f;
        point.y = base.y + sc2::GetRandomScalar() * 15.0f;

        if (++attempt > 150)
            return false;
    } while (!gAPI->query().CanBePlaced(*order_, point)); // TODO: Fix placement for add-ons

    return gHub->AssignBuildTask(order_, point);
}
