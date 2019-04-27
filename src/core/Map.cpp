// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Map.h"
#include "Helpers.h"
#include "Historican.h"
#include "Timer.h"

#include <sc2lib/sc2_search.h>

#include <cmath>

namespace {

const float PI = 3.1415927f;

void CalculateGroundDistances(Expansions& expansions) {
    for (auto& exp : expansions) {
        // Use pathing queries to calculate distance to other expansions
        std::vector<sc2::QueryInterface::PathingQuery> queries;
        queries.reserve(expansions.size());
        for (auto& inner : expansions) {
            if (exp == inner || exp->distanceTo(inner) != 0.0f) // MUST be same skip logic as below
                continue;

            sc2::QueryInterface::PathingQuery query;
            query.start_ = exp->town_hall_location;
            query.end_ = inner->town_hall_location;
            queries.push_back(query);
        }

        // Query SC2 API for distances
        auto results = gAPI->query().PathingDistances(queries);

        // Save down the results
        std::size_t i = 0;
        for (auto& inner : expansions) {
            if (exp == inner || exp->distanceTo(inner) != 0.0f) // MUST be same skip logic as above
                continue;

            exp->ground_distances[inner] = results[i];
            inner->ground_distances[exp] = results[i];
            ++i;
        }
    }
}

void CalculateAirDistances(Expansions& expansions) {
    for (auto& exp : expansions) {
        for (auto& inner : expansions) {
            if (exp->distanceTo(inner) != 0.0f)
                continue;

            float dist = sc2::Distance3D(exp->town_hall_location, inner->town_hall_location);

            exp->ground_distances[inner] = dist;
            inner->ground_distances[exp] = dist;
        }
    }
}

Units GetMineralPatchesAtBase(const sc2::Point3D& baseLocation) {
    return gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                 {IsMineralPatch(), IsWithinDist(baseLocation, 15.0f)}));
}

sc2::Point3D GetCenterBehindMinerals(const sc2::Point3D& baseLocation) {
    // TODO: Reuse Map's Clusters?
    auto mineralPatches = GetMineralPatchesAtBase(baseLocation);

    if (mineralPatches.empty())
        return baseLocation;

    // Get center of resources around base
    sc2::Point3D resourceCenter;
    float maxDist = 0;
    for (auto& mineralPatch : mineralPatches) {
        resourceCenter += mineralPatch->pos;
        float dist = sc2::Distance2D(mineralPatch->pos, baseLocation);
        if (maxDist < dist)
            maxDist = dist;
    }
    resourceCenter /= mineralPatches.size();
    resourceCenter.z = baseLocation.z;

    // Get direction vector
    auto directionVector = resourceCenter - baseLocation;
    sc2::Normalize3D(directionVector);

    return baseLocation + directionVector * (maxDist + 1.5f);
}

}  // namespace

Expansion::Expansion(const sc2::Point3D& town_hall_location_):
    town_hall_location(town_hall_location_), alliance(sc2::Unit::Alliance::Neutral), town_hall(nullptr) {
    center_behind_minerals = GetCenterBehindMinerals(town_hall_location);
}

Expansions CalculateExpansionLocations() {
    auto locations = gAPI->CalculateExpansionLocations();

    Expansions expansions;

    // Manually add our starting location which can never be considered buildable due to having a CC
    auto ccs = gAPI->observer().GetUnits(IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER), sc2::Unit::Alliance::Self);
    if (!ccs.empty()) {
        auto exp = std::make_shared<Expansion>(ccs.front()->pos);
        exp->alliance = sc2::Unit::Alliance::Self;
        expansions.emplace_back(std::move(exp));
    }

    for (const auto& location: locations) {
        auto exp = std::make_shared<Expansion>(location);
        expansions.emplace_back(std::move(exp));
    }

    Timer t;
    t.Start();
#ifndef FAST_STARTUP
    CalculateGroundDistances(expansions);
#else
    CalculateAirDistances(expansions);
#endif
    gHistory.info() << "Calculating distances between expansions took " << t.Finish() << " ms" << std::endl;

    return expansions;
}

std::unique_ptr<Overseer::MapImpl> gOverseerMap;
