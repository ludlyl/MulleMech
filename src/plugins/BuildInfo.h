
#pragma once

#include "Plugin.h"
#include "Dispatcher.h"
#include "../BuildingPlacer.h"

struct BuildInfo: Plugin {

    void OnGameStart(Builder* builder_) final;

    void OnStep(Builder*) final;

private:
    const sc2::ObservationInterface* m_observer;

};

static sc2::Point3D buildingPoint;

static float baseKValue;
