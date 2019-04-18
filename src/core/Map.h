// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Unit.h"

#include <sc2api/sc2_unit.h>

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

// Expansion denotes all possible base locations, occupied or not, on the map (including our main base)
struct Expansion {
    explicit Expansion(const sc2::Point3D& town_hall_location_);

    sc2::Point3D town_hall_location;
    sc2::Unit::Alliance alliance;
    std::unordered_map<std::shared_ptr<Expansion>, float> ground_distances;
    Unit* town_hall; // nullptr if we do not control the base

    // Returns: walk distance to other expansion
    float distanceTo(const std::shared_ptr<Expansion>& other_) const {
        auto itr = ground_distances.find(other_);
        if (itr != ground_distances.end())
            return itr->second;
        return 0.0f;
    }
};

typedef std::vector<std::shared_ptr<Expansion>> Expansions;

// NOTE (alkurbatov): Slightly optimised version of the builtin function.
Expansions CalculateExpansionLocations();
