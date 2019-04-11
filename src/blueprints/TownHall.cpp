// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "TownHall.h"
#include "Hub.h"
#include "core/API.h"

bool bp::TownHall::CanBeBuilt(const Order *order_) {
    for (auto& exp : gHub->GetExpansions()) {
        if (exp->alliance == sc2::Unit::Alliance::Neutral) {
            return gHub->GetClosestFreeWorker(exp->town_hall_location) != nullptr;
        }
    }

    return false;
}

bool bp::TownHall::Build(Order* order_) {
    for (auto& exp : gHub->GetExpansions()) {
        if (exp->alliance == sc2::Unit::Alliance::Neutral) {
            return gHub->AssignBuildTask(order_, exp->town_hall_location);
        }
    }

    return false;
}
