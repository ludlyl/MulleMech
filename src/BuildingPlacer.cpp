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

std::optional<sc2::Point3D> BuildingPlacer::CalculateFreePlaceBehindMinerals(
    const Order &order, const sc2::Point3D baseLocation) {
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

//method calculate point in front of CC, away from mineral line
sc2::Point3D BuildingPlacer::GetPointFrontOfCC(const sc2::Point3D& baseLocation) {
    sc2::Point2D point;

    //TODO:use method instead GetBaseKValue(). Or remove GetBaseKValue()
    std::vector<const sc2::Unit*> posGeysers = GetGeysersPos();

    const sc2::Unit* geyser1 = posGeysers.back();
    posGeysers.pop_back();

    const sc2::Unit* geyser2 = posGeysers.back();
    posGeysers.pop_back();

    sc2::Point2D point1(geyser1->pos.x, geyser1->pos.y);
    sc2::Point2D point2(geyser2->pos.x, geyser2->pos.y);

    sc2::Point2D baseToPoint1 = sc2::operator-(point1, baseLocation);

    float kValue = getKValue(point1, point2);

    sc2::Point2D secondPointInBaseLine = getPointInLine(baseLocation, kValue, 1);
    sc2::Point2D vecInBaseLine = sc2::operator-(secondPointInBaseLine, baseLocation);

    float dotProd = sc2::Dot2D(baseToPoint1, vecInBaseLine);

    float magnitude = sqrtf((vecInBaseLine.x * vecInBaseLine.x) + (vecInBaseLine.y * vecInBaseLine.y));
    float baseComponent = dotProd / magnitude;

    sc2::Point2D normalizedBaseVec = sc2::operator/(vecInBaseLine, magnitude);

    sc2::Point2D finalVecInBaseLine = sc2::operator*(normalizedBaseVec, baseComponent);

    sc2::Point2D doubleVec = sc2::operator*(finalVecInBaseLine, 2);

    sc2::Point2D helperVec = sc2::operator-(doubleVec, baseToPoint1);
    point = sc2::operator-(helperVec, baseToPoint1);

    point.x = point.x + baseLocation.x;
    point.y = point.y + baseLocation.y;

    sc2::Point3D pointFront;
    pointFront.x = point.x;
    pointFront.y = point.y;
    pointFront.z = baseLocation.z;

    return pointFront;
}

//method which gets a random building point through a line in the point in front of CC
std::optional<const sc2::Point3D> BuildingPlacer::FindPlaceInFrontOfCC(const Order& order, const sc2::Point3D baseLocation) {
    unsigned attempt = 0;
    while (attempt < 20) {
        float randomDist = sc2::GetRandomScalar() * 10.0f;
        sc2::Point2D newPoint = getPointInLine(BuildInfo::buildingPoint, BuildInfo::baseKValue, randomDist);
        sc2::Point3D pos = sc2::Point3D(newPoint.x, newPoint.y, baseLocation.z);

        if (gAPI->query().CanBePlaced(order, pos)) {
            return pos;
        }
        attempt++;
    }

    return std::nullopt;
}

float BuildingPlacer::GetBaseKValue() {
    std::vector<const sc2::Unit*> posGeysers = GetGeysersPos();

    const sc2::Unit* geyser1 = posGeysers.back();
    posGeysers.pop_back();

    const sc2::Unit* geyser2 = posGeysers.back();
    posGeysers.pop_back();

    sc2::Point2D point1(geyser1->pos.x, geyser1->pos.y);
    sc2::Point2D point2(geyser2->pos.x, geyser2->pos.y);

    return getKValue(point1, point2);
}

const std::vector<const sc2::Unit*> BuildingPlacer::GetGeysersPos() {
    auto geysers = gAPI->observer().GetUnits(IsFreeGeyser(),
                                             sc2::Unit::Alliance::Neutral);

    return geysers.GetTwoClosestUnits(gAPI->observer().StartingLocation());
}

float BuildingPlacer::getKValue(sc2::Point2D p1, sc2::Point2D p2) {
    return (p2.y-p1.y)/(p2.x-p1.x);
}

sc2::Point2D BuildingPlacer::getPointInLine(sc2::Point2D p1, float kValue, float dist) {
    sc2::Point2D p2;
    p2.x = p1.x + dist;
    p2.y = p1.y +  (p2.x - p1.x) * kValue;
    return p2;
}
