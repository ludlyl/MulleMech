// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Unit.h"

#include <sc2api/sc2_unit.h>
#include <overseer/MapImpl.h>

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

// Expansion denotes all possible base locations, occupied or not, on the map (including our main base)
struct Expansion {
    explicit Expansion(const sc2::Point3D& town_hall_location_);

    // TODO: Some of these values should be const (it's bad if other parts of the code can change all of these)
    sc2::Point3D town_hall_location;
    sc2::Point3D center_behind_minerals;
    sc2::Unit::Alliance alliance;
    std::unordered_map<std::shared_ptr<Expansion>, float> ground_distances;
    Unit* town_hall;
    std::vector<sc2::Point2D> geysers_positions; // Note: For some reason the data on these do not update
    std::vector<Unit*> refineries; // Only set for our own bases as of now

    // Returns: walk distance to other expansion
    float distanceTo(const std::shared_ptr<Expansion>& other_) const {
        auto itr = ground_distances.find(other_);
        if (itr != ground_distances.end())
            return itr->second;
        return 0.0f;
    }
};

typedef std::vector<std::shared_ptr<Expansion>> Expansions;

// Better version than the built in
Expansions CalculateExpansionLocations();

extern std::unique_ptr<Overseer::MapImpl> gOverseerMap;

