// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct RepairMan : Plugin {
    void OnStep(Builder* builder_) final;

    void OnUnitDestroyed(Unit* unit_, Builder* builder_) final;

private:
    //std::vector<std::pair<Unit, std::vector<Unit>>> m_workerRepairing;
    //std::vector<std::pair<Unit, Unit>> m_workerRepairTarget;
    int repairMen;

    Unit* m_self;

    std::size_t CountRepairMen(Unit* unit);

    bool IsAnyRepairersNearby(Unit* unit);
};
