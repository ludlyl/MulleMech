#include "BuildingPlacer.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Map.h"
#include "Hub.h"

#include <sc2api/sc2_common.h>

void BuildingPlacer::OnGameStart() {
    // TODO: Mark mining path as reserved!
    // TODO: Free mining path once minerals & gas are mined out
    // TODO: Expansion locations should be marked with a special flag

    // NOTE: This way of handling/wrapping regions does NOT support region merging
    // Should already be sorted by region id as the underlying data structure for regions is a std::map
    sc2::Point2DI point; // Used to reuse the point object
    for (const auto& region : gOverseerMap->getRegions()) {
        RegionWrapper regionWrapper(region);
        for (const auto& tile : region->getTilePositions()) {
            // Note: As we don't add minerals here the mineral locations will never be buildable! Might want to fix this
            if (tile.second->getTileTerrain() == Overseer::TileTerrain::buildAndPath
                || tile.second->getTileTerrain() == Overseer::TileTerrain::build) {
                // tile x & y should already be integers, might want to assert if they aren't
                point.x = static_cast<int>(tile.first.x);
                point.y = static_cast<int>(tile.first.y);
                regionWrapper.buildable_tiles[point] = tile.second;

                if (regionWrapper.min_x > point.x)
                    regionWrapper.min_x = point.x;
                else if (regionWrapper.max_x < point.x)
                    regionWrapper.max_x = point.x;

                if (regionWrapper.min_y > point.y)
                    regionWrapper.min_y = point.y;
                else if (regionWrapper.max_y < point.y)
                    regionWrapper.max_y = point.y;
            }
        }
        regions.emplace_back(std::move(regionWrapper));
    }
}

void BuildingPlacer::OnUnitCreated(const Unit* unit_) {
    if (IsBuilding()(*unit_) && !unit_->is_flying) {
        AddBuildingToOccupiedTiles(unit_, TileOccupationStatus::has_building);
    }
}

void BuildingPlacer::OnUnitDestroyed(const Unit* unit_) {
    // TODO: This only gets called if we observe the unit being destroyed.
    //  When/if the enemy kills his own units or if they burn down outside our vision the tiles will never be freed
    // TODO: We don't know if the building has reserved space for an add-on (and if so we currently don't free that space)
    if (IsBuilding()(*unit_) && !unit_->is_flying) {
        RemoveBuildingFromOccupiedTiles(unit_);
    }
}

void BuildingPlacer::OnUnitEnterVision(const Unit* unit_) {
    // The same building will be added over an over again...
    if (IsBuilding()(*unit_) && !unit_->is_flying) {
        AddBuildingToOccupiedTiles(unit_, TileOccupationStatus::has_building);
    }
}

std::optional<sc2::Point3D> BuildingPlacer::ReserveBuildingSpace(const Order& order_, bool include_addon_space_) {
    assert(IsBuilding()(order_.unit_type_id));
    if (include_addon_space_) {
        assert(IsBuildingWithSupportForAddon()(order_.unit_type_id));
    }

    // We currently have 4 different cases:
    //      Depot (can be placed anywhere as they can be lowered)
    //      Production building with addon
    //      Expansions (will fail/turn into normal building if no free expansion place exists)
    //      Normal building (we will want 2x2 of free space around the whole building)

    // The logic for where a (non-expansion) building is placed is currently very simple:
    // Try to place at the first free expansion/region sorted from distance to our main
    // (we don't take into account if we own it or not, if enemy units are nearby etc.)
    // Start searching in the bottom left corner of the region
    // (would be better to start in the corner closest to the town hall location,
    // use a spiral from town hall location or even region center)
    // If we fail we just increment with 1 tile at a time, this is HIGHLY inefficient and should be improved
    // (largest distance to unpathable could probably be used)!
    // Lastly we check if the building is actually placeable with the api (and if not we continue to search)
    // TODO: We need to check if the worker can actually reach the placement tile too!

    auto radius = gAPI->observer().GetAbilityData(order_.ability_id).footprint_radius;
    int width = static_cast<int>(radius * 2);
    int height = width;
    int margin;
    sc2::Point2DI point; // Used to reuse the point object
    sc2::Point2DI addon_bottom_left_tile; // Only used when include_addon_space_ is true
    // TODO: We want some kind of "search origin" that is different for different kinds of buildings.
    //  I.e. we want depots, production buildings, ebays/armories in different places

    // Special handling for town halls. Will try to be placed at the first free expansion location
    // (and if all fails it will be treated as a normal building)
    // TODO: As this doesn't leave any margin around the town hall this risks blocking of an area completely
    //  (even if the chance for it to happen isn't that big)
    // Some alternatives to fix this are:
    //      Require free space around the town hall
    //      Add a "reserved_for_town_hall" to TileOccupationStatus and make sure no
    //      buildings are placed inside of or next to the town hall location
    if (order_.unit_type_id == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER) {
        for (const auto& expansion : gHub->GetExpansions()) {
            if (expansion->alliance == sc2::Unit::Alliance::Neutral) {
                // The "buildable_tiles" aren't really needed when we try to place town halls at expansions.
                // Could create a IsBuildSpaceFree that doesn't require buildable_tiles to be passed in..
                const auto& closest_region = gOverseerMap->getNearestRegion(expansion->town_hall_location); // Is this costly?
                auto& wrapped_region = regions[closest_region->getId() -1]; // Might be bad to assume that the regions remain unchanged
                // Might want to assert if these aren't integers
                point.x = static_cast<int>(expansion->town_hall_location.x - radius);
                point.y = static_cast<int>(expansion->town_hall_location.y - radius);

                if (IsBuildSpaceFree(point, width, height, wrapped_region.buildable_tiles)) {
                    if (gAPI->query().CanBePlaced(order_, expansion->town_hall_location)) {
                        MarkTilesAsReserved(point, width, height);
                        return expansion->town_hall_location;
                    }
                }
            }
        }
    }

    if (order_.unit_type_id == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
        // We don't need margin around depots as they can be lowered
        margin = 0;
    } else {
        // We use DefaultBuildingMargin around everything else
        margin = DefaultBuildingMargin;
    }

    for (const auto& expansion : gHub->GetExpansions()) {
        const auto& closest_region = gOverseerMap->getNearestRegion(expansion->town_hall_location); // Is this costly?
        auto& wrapped_region = regions[closest_region->getId() - 1]; // Might be bad to assume that the regions remain unchanged

        for (int x = wrapped_region.min_x; x <= wrapped_region.max_x; x++) {
            for (int y = wrapped_region.min_y; y <= wrapped_region.max_y; y++) {
                point.x = x;
                point.y = y;
                // We need to multiple the margin by 2 as we want margin on all sides
                if (IsBuildSpaceFree(point, width + margin * 2, height + margin * 2, wrapped_region.buildable_tiles)) {
                    if (include_addon_space_) {
                        addon_bottom_left_tile = point;
                        addon_bottom_left_tile.x += width;
                        if (IsBuildSpaceFree(addon_bottom_left_tile, AddonSize + margin * 2, AddonSize + margin * 2, wrapped_region.buildable_tiles)) {
                            // When we actually place the addon we don't want to check with the margin anymore
                            addon_bottom_left_tile.x += margin;
                            addon_bottom_left_tile.y += margin;
                        } else {
                            continue;
                        }
                    }
                    // When we actually place the building we don't want to check with the margin anymore
                    point.x += margin;
                    point.y += margin;
                    sc2::Point3D pos(point.x + radius, point.y + radius, wrapped_region.region->getTilePositions().front().second->getTerrainHeight());
                    // Do we want to do "CanBePlaced" for the addon too? Doing it has both pros and cons
                    if (gAPI->query().CanBePlaced(order_, pos)) {
                        MarkTilesAsReserved(point, width, height);
                        if (include_addon_space_) {
                            MarkTilesAsReserved(addon_bottom_left_tile, AddonSize, AddonSize);
                        }
                        return pos;
                    }
                }
            }
        }
    }

    return std::nullopt;
}

void BuildingPlacer::FreeReservedBuildingSpace(sc2::Point3D building_position_, sc2::UNIT_TYPEID building_type_,
                                               bool included_addon_space_) {
    bool is_building = IsBuilding()(building_type_);
    assert(is_building);
    if (included_addon_space_) {
        assert(IsBuildingWithSupportForAddon()(building_type_));
    }

    auto ability_id = gAPI->observer().GetUnitTypeData(building_type_).ability_id;
    auto radius = gAPI->observer().GetAbilityData(ability_id).footprint_radius;
    // Should always be whole numbers
    int left_side_x = static_cast<int>(building_position_.x - radius);
    int bottom_side_y = static_cast<int>(building_position_.y - radius);
    int width = static_cast<int>(radius * 2);
    int height = width;
    sc2::Point2DI point; // Used to reuse the point object

    for (int x  = left_side_x; x < (left_side_x + width); x++) {
        for (int y  = bottom_side_y; y < (bottom_side_y + height); y++) {
            point.x = x;
            point.y = y;
            auto itr = occupied_tiles.find(point);
            if (itr != occupied_tiles.end() && itr->second == BuildingPlacer::TileOccupationStatus::reserved) {
                occupied_tiles.erase(itr);
            } else {
                assert(false && "Tried to free non reserved building space");
            }
        }
    }
    // This is a unnecessarily inefficient way of solving it
    if (included_addon_space_) {
        auto addon_position = (GetTerranAddonPosition(building_position_));
        // Add-ons are sometimes bugged (e.g. when checking if they can be placed)
        // so we use another 2x2 building here instead
        FreeReservedBuildingSpace(sc2::Point3D(addon_position.x, addon_position.y, building_position_.z),
                                  sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, false);
    }
}

bool BuildingPlacer::IsGeyserUnoccupied(const Unit* geyser_) const {
    assert(IsGeyser()(*geyser_));

    auto radius = geyser_->radius - 0.25f; // Unit radius seem to always be 0.25 bigger for some reason...
    int width = static_cast<int>(radius * 2);
    int height = width;
    sc2::Point2DI bottom_left_tile;
    bottom_left_tile.x = static_cast<int>(geyser_->pos.x - radius);
    bottom_left_tile.y = static_cast<int>(geyser_->pos.y - radius);
    sc2::Point2DI point; // Used to reuse the point object
    for (int x = bottom_left_tile.x; x < (bottom_left_tile.x + width); x++) {
        for (int y = bottom_left_tile.y; y < (bottom_left_tile.y + height); y++) {
            point.x = x;
            point.y = y;
            if (occupied_tiles.count(point) > 0) {
                return false;
            }
        }
    }
    return true;
}

bool BuildingPlacer::ReserveGeyser(const Unit* geyser_) {
    if (IsGeyserUnoccupied(geyser_)) {
        auto radius = geyser_->radius - 0.25f; // Unit radius seem to always be 0.25 bigger for some reason...
        int width = static_cast<int>(radius * 2);
        int height = width;
        sc2::Point2DI bottom_left_tile;
        bottom_left_tile.x = static_cast<int>(geyser_->pos.x - radius);
        bottom_left_tile.y = static_cast<int>(geyser_->pos.y - radius);
        MarkTilesAsReserved(bottom_left_tile, width, height);
        return true;
    }

    return false;
}

void BuildingPlacer::AddBuildingToOccupiedTiles(const Unit* unit_, TileOccupationStatus tile_occupation_status_) {
    bool is_building = IsBuilding()(*unit_);
    assert(is_building);
    if (unit_->is_flying)
        return;

    auto radius = unit_->radius - 0.25f; // Unit radius seem to always be 0.25 bigger for some reason...
    // Should always be whole numbers
    int left_side_x = static_cast<int>(unit_->pos.x - radius);
    int bottom_side_y = static_cast<int>(unit_->pos.y - radius);
    int width = static_cast<int>(radius * 2);
    int height = width;
    sc2::Point2DI point; // Used to reuse the point object

    for (int x  = left_side_x; x < (left_side_x + width); x++) {
        for (int y  = bottom_side_y; y < (bottom_side_y + height); y++) {
            point.x = x;
            point.y = y;
            occupied_tiles[point] = tile_occupation_status_;
        }
    }
}

void BuildingPlacer::RemoveBuildingFromOccupiedTiles(const Unit* unit_) {
    bool is_building = IsBuilding()(*unit_);
    assert(is_building);
    if (unit_->is_flying)
        return;

    auto radius = unit_->radius - 0.25f; // Unit radius seem to always be 0.25 bigger for some reason...
    // Should always be whole numbers
    int left_side_x = static_cast<int>(unit_->pos.x - radius);
    int bottom_side_y = static_cast<int>(unit_->pos.y - radius);
    int width = static_cast<int>(radius * 2);
    int height = width;
    sc2::Point2DI point; // Used to reuse the point object

    for (int x  = left_side_x; x < (left_side_x + width); x++) {
        for (int y  = bottom_side_y; y < (bottom_side_y + height); y++) {
            point.x = x;
            point.y = y;
            occupied_tiles.erase(point);
        }
    }
}

bool BuildingPlacer::IsBuildSpaceFree(const sc2::Point2DI& bottom_left_tile_, const int width_, const int height_,
                                      const std::unordered_map<sc2::Point2DI, std::shared_ptr<Overseer::Tile>, Point2DIHasher>& buildable_tiles_) const {
    sc2::Point2DI point; // Used to reuse the point object
    for (int x = bottom_left_tile_.x; x < (bottom_left_tile_.x + width_); x++) {
        for (int y = bottom_left_tile_.y; y < (bottom_left_tile_.y + height_); y++) {
            point.x = x;
            point.y = y;
            if (occupied_tiles.count(point) > 0 || buildable_tiles_.count(point) == 0) {
                return false;
            }
        }
    }
    return true;
}

void BuildingPlacer::MarkTilesAsReserved(const sc2::Point2DI& bottom_left_tile_, const int width_, const int height_) {
    sc2::Point2DI point; // Used to reuse the point object
    for (int x  = bottom_left_tile_.x; x < (bottom_left_tile_.x + width_); x++) {
        for (int y  = bottom_left_tile_.y; y < (bottom_left_tile_.y + height_); y++) {
            point.x = x;
            point.y = y;
            const auto& result = occupied_tiles.insert({point, TileOccupationStatus::reserved});
            assert(result.second && "Tried to mark already occupied tile as reserved");
        }
    }
}

std::unique_ptr<BuildingPlacer> gBuildingPlacer;
