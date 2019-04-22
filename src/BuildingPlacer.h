#pragma once

#include "core/Order.h"
#include "core/Units.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <overseer/Region.h>

#include <optional>
#include <limits>

class BuildingPlacer {
public:
    void OnGameStart();

    void OnUnitCreated(const Unit* unit_);

    void OnUnitDestroyed(const Unit* unit_);

    void OnUnitEnterVision(const Unit* unit_);

    /*
     * We want some more types of "ReserveBuildingSpace functions". Some examples are:
     * ReserveRecommendedBuildingSpace (chooses a fitting function)
     * ReserveNormalBuildingSpace
     * ReserveProductionBuildingSpace (i.e. building space with addon)
     * ReserveExpansionsBuildingSpace
     * ReserveWallBuildingSpace -   Might want to be able to pass in a base/expansions/region here.
     *                              For there to be any point of having this logic for supply depots
     *                              lowering/raising needs to be implemented first though.
     * ReserveDefensiveBuildingSpace
     * ReserveRegionEdgeBuildingSpace (requires an expansion to be passed in)
     */

    // Note: This doesn't support refineries
    std::optional<sc2::Point3D> ReserveBuildingSpace(const Order& order_, bool include_addon_space_ = false);

private:
    struct Point2DIHasher {
        std::size_t operator()(const sc2::Point2DI& k) const {
            // Knuth's hash
            auto return_value = static_cast<size_t>(k.x);
            return_value *= 2654435761U;
            return return_value ^ static_cast<size_t>(k.y);
        }
    };

    struct RegionWrapper {
        explicit RegionWrapper(std::shared_ptr<Overseer::Region> region_) : region(std::move(region_)) {}
        const std::shared_ptr<Overseer::Region> region;
        std::unordered_map<sc2::Point2DI, std::shared_ptr<Overseer::Tile>, Point2DIHasher> buildable_tiles;
        // Setting min x/y to int max is not very pretty
        // (initializing these with the value from the first tile in the region
        // or even map tile width/height would look better), but it works..
        int min_x = std::numeric_limits<int>::max();
        int max_x = 0;
        int min_y = std::numeric_limits<int>::max();
        int max_y = 0;
    };

    enum class TileOccupationStatus {
        reserved,
        has_building
        // has_enemy_building?
    };

    void AddBuildingToOccupiedTiles(const Unit* unit_, TileOccupationStatus tile_occupation_status_);

    void RemoveBuildingFromOccupiedTiles(const Unit* unit_);

    // Checks both so the tiles are unoccupied, buildable and inside the region.
    // No margin is added inside the function so if margin around the building
    // is wanted pass that in as part of the dimensions.
    // Region could be calculated from the bottom_left_tile but that would be a bit unnecessary
    // Would it be better to pass in a RegionWrapper reference (or just id?)
    bool IsBuildSpaceFree(const sc2::Point2DI& bottom_left_tile_, int width_, int height_,
                          const std::unordered_map<sc2::Point2DI, std::shared_ptr<Overseer::Tile>, Point2DIHasher>& buildable_tiles_);

    // IsBuildSpaceFree should always be called before this is called (or the tiles should be checked at least)
    // This will assert/throw an error if the any of the tiles are already marked as occupied
    void MarkTilesAsReserved(const sc2::Point2DI& bottom_left_tile_, int width_, int height_);

    // The position of the RegionWrapper in the vector is the region id
    std::vector<RegionWrapper> regions;
    // Would it be better to use one "occupied_tiles" and one "reserved_tiles" (could be unordered_sets or 2d arrays)?
    std::unordered_map<sc2::Point2DI, TileOccupationStatus, Point2DIHasher> occupied_tiles;

    static constexpr int DefaultBuildingMargin = 2;
    static constexpr int AddonSize = 2; // Both width and height
};

extern std::unique_ptr<BuildingPlacer> gBuildingPlacer;
