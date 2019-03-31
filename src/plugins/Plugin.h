// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "core/Unit.h"

struct Plugin {
    virtual ~Plugin() {
    }

    virtual void OnGameStart(Builder*) {
    }

    virtual void OnStep(Builder*) = 0;

    virtual void OnUnitCreated(Unit*) {
    }

    virtual void OnUnitDestroyed(Unit*, Builder*) {
    }

    virtual void OnBuildingConstructionComplete(Unit*) {
    }

    virtual void OnUnitIdle(Unit*, Builder*) {
    }

    virtual void OnUpgradeCompleted(sc2::UpgradeID) {
    }

    virtual void OnUnitEnterVision(Unit*) {

    }

    virtual void OnGameEnd() {
    }
};
