// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Map.h"
#include "Helpers.h"
#include "Historican.h"
#include "Timer.h"

#include <sc2lib/sc2_search.h>

#include <cmath>
#include <optional>
#include <queue>

namespace {

constexpr int SearchMinOffset = -10;    // Defines min x and y in rectangle used for 
constexpr int SearchMaxOffset = 10;
constexpr sc2::ABILITY_ID TestAbility = sc2::ABILITY_ID::BUILD_COMMANDCENTER;
constexpr float DistanceErrorMargin = 10.0f;
constexpr float PatchNeighborDistance = 5.0f; // Maximum distance between neighboring mineral patches

// A line of minerals together with point where Expansion can be built
struct MineralLine {
    explicit MineralLine(const sc2::Point3D& initial_patch_);

    void AddMineralPatch(const sc2::Point3D& point_);

    float Height() const;

    sc2::Point2D Center() const;

    std::optional<sc2::Point2D> town_hall_location;      // Saved town hall location
    std::vector<sc2::Point3D> mineral_patches;           // Points of mineral patches
};

MineralLine::MineralLine(const sc2::Point3D& initial_patch_) {
    mineral_patches.reserve(8);
    mineral_patches.push_back(initial_patch_);
}

void MineralLine::AddMineralPatch(const sc2::Point3D& point_) {
    mineral_patches.push_back(point_);
}

float MineralLine::Height() const {
    if (mineral_patches.empty())
        return 0.0f;

    return mineral_patches.back().z;
}

sc2::Point2D MineralLine::Center() const {
    assert(!mineral_patches.empty());

    float x = 0;
    float y = 0;
    for (auto& mineral : mineral_patches) {
        x += mineral.x;
        y += mineral.y;
    }

    return sc2::Point2D(x / mineral_patches.size(), y / mineral_patches.size());
}

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

std::vector<MineralLine> GetMineralLines() {
    auto mineral_patches = gAPI->observer().GetUnits(
        [](const auto& unit) {
        return unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD || unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750 ||
            unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD || unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750 ||
            unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD || unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750 ||
            unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD || unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750 ||
            unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD || unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750 ||
            unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD || unit.unit_type == sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750;
    });

    // Find all mineral lines (rather inefficient method, but guarantees no accidents,
    // such as thinking two bases are linked, or ending up with one base being considered as two)
    std::vector<MineralLine> mineral_lines;
    mineral_lines.reserve(16);
    while (!mineral_patches.empty()) {
        std::queue<sc2::Point3D> mineral_frontier;
        mineral_frontier.push(mineral_patches.front()->pos);
        MineralLine line(mineral_frontier.front());
        mineral_patches.erase(mineral_patches.begin());

        // Gather mineral nodes by continously considering the closest remaining
        while (!mineral_patches.empty() && !mineral_frontier.empty()) {
            auto mineral_pos = mineral_frontier.front();
            auto closest_patch = mineral_patches.GetClosestUnit(mineral_pos);
            auto distance = Distance2D(closest_patch->pos, mineral_pos);

            if (distance >= PatchNeighborDistance) {
                mineral_frontier.pop();
                continue;
            }

            mineral_frontier.push(closest_patch->pos);
            line.AddMineralPatch(closest_patch->pos);
            mineral_patches.remove(closest_patch);
        }

        mineral_lines.push_back(line);
    }

    gHistory.info() << "Map contains " << mineral_lines.size() << " mineral lines" << std::endl;

    return mineral_lines;
}

void CalculateGeysers(Expansions& expansions){
    auto geysers = gAPI->observer().GetUnits(IsGeyser());

    // Not all bases has 2 geysers, so we can't just do GetClosestUnit(exp.pos)
    for (auto& geyser : geysers) {
        // This is pretty inefficient
        auto closest = expansions[0];
        for (auto& exp : expansions) {
            if (sc2::DistanceSquared2D(geyser->pos, exp->town_hall_location) <
                sc2::DistanceSquared2D(geyser->pos, closest->town_hall_location)) {
                closest = exp;
            }
        }
        closest->geysers.emplace_back(geyser);
    }
}

}  // namespace

Expansion::Expansion(const sc2::Point3D& town_hall_location_):
    town_hall_location(town_hall_location_), alliance(sc2::Unit::Alliance::Neutral), town_hall(nullptr) {
    center_behind_minerals = GetCenterBehindMinerals(town_hall_location);
}

Expansions CalculateExpansionLocations() {
    auto mineral_lines = GetMineralLines();

    if (mineral_lines.empty()) {
        gHistory.error() << "No expansion locations could be found!" << std::endl;
        return Expansions();
    }

    Expansions expansions;

    // Manually register our starting location (and remove its mineral line from consideration)
    auto ccs = gAPI->observer().GetUnits(IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER), sc2::Unit::Alliance::Self);
    if (!ccs.empty()) {
        for (auto itr = mineral_lines.begin(); itr != mineral_lines.end(); ++itr) {
            if (sc2::Distance2D(itr->Center(), ccs.front()->pos) < DistanceErrorMargin) {
                auto expo = std::make_shared<Expansion>(ccs.front()->pos);
                expo->alliance = sc2::Unit::Alliance::Self;
                expo->town_hall = ccs.front();
                expansions.emplace_back(std::move(expo));
                mineral_lines.erase(itr);
                break;
            }
        }
    }

    // Find TownHall position for all mineral lines
    for (auto& line : mineral_lines) {
        // Expansions are always built at (x+0.5, y+0.5) if (x, y) is a pair of ints
        auto center = line.Center();
        center.x = static_cast<int>(center.x) + 0.5f;
        center.y = static_cast<int>(center.y) + 0.5f;

        // Find all possible TownHall locations for MineralLine
        std::vector<sc2::QueryInterface::PlacementQuery> queries;
        queries.reserve((SearchMaxOffset - SearchMinOffset + 1) * (SearchMaxOffset - SearchMinOffset + 1));
        for (int x_offset = SearchMinOffset; x_offset <= SearchMaxOffset; ++x_offset) {
            for (int y_offset = SearchMinOffset; y_offset <= SearchMaxOffset; ++y_offset) {
                sc2::Point2D pos(center.x + x_offset, center.y + y_offset);
                queries.emplace_back(TestAbility, pos);
            }
        }
        auto results = gAPI->query().CanBePlaced(queries);

        // Narrow it down to the best TownHall position for MineralLine
        for (int x_offset = SearchMinOffset; x_offset <= SearchMaxOffset; ++x_offset) {
            for (int y_offset = SearchMinOffset; y_offset <= SearchMaxOffset; ++y_offset) {
                sc2::Point2D pos(center.x + x_offset, center.y + y_offset);

                // Buildable?
                int index = (x_offset + 0 - SearchMinOffset) * (SearchMaxOffset - SearchMinOffset + 1) + (y_offset + 0 - SearchMinOffset);
                assert(0 <= index && index < static_cast<int>(results.size()));
                if (!results[static_cast<std::size_t>(index)])
                    continue;

                if (line.town_hall_location.has_value()) {
                    if (sc2::DistanceSquared2D(center, pos) <
                        sc2::DistanceSquared2D(center, line.town_hall_location.value())) {
                        line.town_hall_location = pos;
                    }
                } else {
                    line.town_hall_location = pos;
                }
            }
        }

        // Add Expansion entry
        auto p = line.town_hall_location.value();
        expansions.emplace_back(std::make_shared<Expansion>(sc2::Point3D(p.x, p.y, line.Height())));
    }

    CalculateGroundDistances(expansions);
    CalculateGeysers(expansions);

    return expansions;
}

std::unique_ptr<Overseer::MapImpl> gOverseerMap;
