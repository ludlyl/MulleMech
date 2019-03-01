#include "BuildingPlacer.h"

#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_common.h>

sc2::Point3D BuildingPlacer::GetCenterBehindMinerals(const sc2::Point3D baseLocation) {
    // TODO: Reuse Map's Clusters?
    auto resources = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
        {IsVisibleMineralPatch(), IsWithinDist(baseLocation, 15.0f)}))();

    if (resources.empty())
        return baseLocation;

    // Get center of resources around base
    sc2::Point3D resourceCenter;
    float maxDist = 0;
    for (auto& resource : resources) {
        resourceCenter += resource->pos;
        float dist = sc2::Distance2D(resource->pos, baseLocation);
        if (maxDist < dist)
            maxDist = dist;
    }
    resourceCenter /= resources.size();
    resourceCenter.z = baseLocation.z;

    // Get direction vector
    auto directionVector = resourceCenter - baseLocation;
    sc2::Normalize3D(directionVector);

    return baseLocation + directionVector * (maxDist + 1.5f);
}

std::optional<sc2::Point3D> BuildingPlacer::PlaceBehindMinerals(const Order& order, const sc2::Point3D baseLocation) {
    auto center = GetCenterBehindMinerals(baseLocation);
    auto startDirection = sc2::Point2D(center - baseLocation);
    sc2::Normalize2D(startDirection);
    float startDist = sc2::Distance3D(center, baseLocation) - 1.0f;

    float dist = startDist;
    float currentAngleAdjust = 0, leftAngleAdjust = 0, rightAngleAdjust = 0;
    while (true) {
        // Try placing
        auto offset = Rotate2D(startDirection, currentAngleAdjust) * dist;
        auto pos = baseLocation + sc2::Point3D(offset.x, offset.y, baseLocation.z);
        if (gAPI->query().CanBePlaced(order, pos))
            return pos;
        // Failed? Increase dist
        dist += 1.0f;
        // Too far? adjust angle
        if (dist > startDist + 3.0f) {
            if (leftAngleAdjust > 0.6f && rightAngleAdjust > 0.6f)
                break; // Stop trying, no more reasonable space
            if (leftAngleAdjust > rightAngleAdjust) {
                rightAngleAdjust += 0.2f;
                currentAngleAdjust = -rightAngleAdjust;
            } else {
                leftAngleAdjust += 0.2f;
                currentAngleAdjust = leftAngleAdjust;
            }
            dist = startDist;
        }
    }

    return std::nullopt;
}
