
#include "core/API.h"
#include "BuildInfo.h"
#include "../Historican.h"

void BuildInfo::OnGameStart(Builder*) {
    //find point to building line
    buildingPoint = BuildingPlacer::GetPointFrontOfCC(gAPI->observer().StartingLocation());
    //find direction for building line
    baseKValue = BuildingPlacer::GetBaseKValue();
}

void BuildInfo::OnStep(Builder*) {
}

//Define the static variables
sc2::Point3D BuildInfo::buildingPoint = sc2::Point3D();

float BuildInfo::baseKValue = 0.0f;
