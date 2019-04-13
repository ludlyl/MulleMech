#include "BuildingPlacer.h"

#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_common.h>

sc2::Point3D BuildingPlacer::GetCenterBehindMinerals(const sc2::Point3D& baseLocation) {
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

std::optional<sc2::Point3D> BuildingPlacer::CalculateFreePlaceBehindMinerals(
    const Order &order, const sc2::Point3D& baseLocation) {
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

// TODO: This should either not be dependent on the gas positions at all, or it should be able to handle all different kind of gas positions
sc2::Point3D BuildingPlacer::GetPointInFrontOfTownHall(const sc2::Point3D& baseLocation) {
    Units geysers = GetVisibleGeysersAtBase(baseLocation);

    // Only bases with 2 geysers are currently supported
    if (geysers.size() != 2) {
        // TODO: Handle this in some better way than returning a point with {0, 0, 0}
        return sc2::Point3D{};
    }

    sc2::Point2D geyserOnePos(geysers.at(0)->pos.x, geysers.at(0)->pos.y);
    // Vector between CC and a geyser
    sc2::Point2D baseToOneGeyser = geyserOnePos - baseLocation;
    // Rate of change of y in each x-step. Between geysers in HQ
    float kValue = GetBaseKValue(baseLocation);
    // Retrieves a point located in same line as HQs CC. Same direction of line as the geysers line.
    sc2::Point2D pointInBaseLine = GetPointInLine(baseLocation, kValue, 1);
    sc2::Point2D vecInBaseLine = pointInBaseLine - baseLocation;

    // Vector projection of a vector from HQs CC to a geyser (a) onto a vector in line as HQs CC and with same direction as the geyser line (b)
    // a scalar b: a dot b
    float dotProd = sc2::Dot2D(baseToOneGeyser, vecInBaseLine);
    // size of b: |b|
    float magnitude = sqrtf((vecInBaseLine.x * vecInBaseLine.x) + (vecInBaseLine.y * vecInBaseLine.y));
    // size of vector a in vector b's direction: (a dot b)/|b|
    float baseComponent = dotProd / magnitude;

    // Unit vector in baseLine direction
    sc2::Point2D normalizedBaseVec = vecInBaseLine / magnitude;
    // Projected vector = unit vector * size of vector a in vector b's direction
    sc2::Point2D projectedBaseLineVec = normalizedBaseVec * baseComponent;

    // Additions of vectors to get point in front of HQs CC
    sc2::Point2D doubleVec = projectedBaseLineVec * 2;
    sc2::Point2D helperVec = doubleVec - baseToOneGeyser;
    sc2::Point2D pointFrontOfCC2D = helperVec - baseToOneGeyser;

    // Move vector relatively to CC
    pointFrontOfCC2D.x = pointFrontOfCC2D.x + baseLocation.x;
    pointFrontOfCC2D.y = pointFrontOfCC2D.y + baseLocation.y;

    // Transform 2D point to 3D point
    sc2::Point3D pointFront;
    pointFront.x = pointFrontOfCC2D.x;
    pointFront.y = pointFrontOfCC2D.y;
    pointFront.z = baseLocation.z;

    return pointFront;
}

std::optional<sc2::Point3D> BuildingPlacer::CalculateFreePlaceInFrontOfTownHall(const Order& order,
                                                                                const sc2::Point3D& baseLocation,
                                                                                bool includeSpaceForAddon) {
    // As doing "CanBePlaced" is bugged on add-ons, we use another 2x2 building to check it instead
    Order supplyDepotOrder(gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT));

    int attempt = 0;
    // TODO: These values should be cached
    auto pointInFrontOfTownHall = GetPointInFrontOfTownHall(baseLocation);
    auto baseKValue = GetBaseKValue(baseLocation);
    // TODO: This shouldn't be random, instead start from the point closest to the town hall and increase until a free spot is found
    while (attempt < 20) {
        float randomDist = sc2::GetRandomScalar() * 15.0f;
        sc2::Point2D newPoint = GetPointInLine(pointInFrontOfTownHall, baseKValue, randomDist);
        sc2::Point3D pos = sc2::Point3D(newPoint.x, newPoint.y, baseLocation.z);

        if (gAPI->query().CanBePlaced(order, pos)) {
            if (includeSpaceForAddon) {
                if (gAPI->query().CanBePlaced(supplyDepotOrder, GetTerranAddonPosition(pos))) {
                    return pos;
                }
            } else {
                return pos;
            }
        }
        attempt++;
    }

    return std::nullopt;
}

float BuildingPlacer::GetBaseKValue(const sc2::Point3D& baseLocation) {
    Units geysers = GetVisibleGeysersAtBase(baseLocation);

    // Calculating k-value for bases with less than or more than 2 gases isn't supported at the moment
    if (geysers.size() != 2) {
        return 0.f;
    }

    Unit* geyser1 = geysers.at(0);
    Unit* geyser2 = geysers.at(1);

    sc2::Point2D point1(geyser1->pos.x, geyser1->pos.y);
    sc2::Point2D point2(geyser2->pos.x, geyser2->pos.y);

    return GetKValue(point1, point2);
}

Units BuildingPlacer::GetVisibleMineralPatchesAtBase(const sc2::Point3D& baseLocation) {
    return gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                 {IsVisibleMineralPatch(), IsWithinDist(baseLocation, 15.0f)}));
}

Units BuildingPlacer::GetMineralPatchesAtBase(const sc2::Point3D& baseLocation) {
    return gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                {IsMineralPatch(), IsWithinDist(baseLocation, 15.0f)}));
}

Units BuildingPlacer::GetVisibleGeysersAtBase(const sc2::Point3D& baseLocation) {
    return gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                 {IsVisibleGeyser(), IsWithinDist(baseLocation, 15.0f)}));
}

float BuildingPlacer::GetKValue(const sc2::Point2D& p1, const sc2::Point2D& p2) {
    return (p2.y-p1.y)/(p2.x-p1.x);
}

sc2::Point2D BuildingPlacer::GetPointInLine(const sc2::Point2D& p1, float kValue, float dist) {
    sc2::Point2D p2;
    p2.x = p1.x + dist;
    p2.y = p1.y +  (p2.x - p1.x) * kValue;
    return p2;
}
