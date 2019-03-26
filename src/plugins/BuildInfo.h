
#pragma once

#include "Plugin.h"
#include "Dispatcher.h"
#include "core/API.h"
#include "../BuildingPlacer.h"

struct BuildInfo: Plugin {

    void OnGameStart(Builder* builder_) final;

    void OnStep(Builder*) final;

    static sc2::Point3D buildingPoint;

    static float baseKValue;
};
