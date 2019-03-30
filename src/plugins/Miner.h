// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"
#include "core/Units.h"
#include "core/Map.h"

#include <unordered_map>

struct Miner : Plugin {
    void OnStep(Builder* builder_) final;

    void OnUnitCreated(Unit* unit_) final;

    void OnUnitDestroyed(Unit* unit_, Builder*) final;

    void OnUnitIdle(Unit* unit_, Builder*) final;

private:
    // Send workers to mine at another base (expansionDied=false => Only miners get redistributed)
    void SplitWorkersOf(const std::shared_ptr<Expansion>& expansion, bool expansionDied);

    // Balance Workers by means of redistribution
    void BalanceWorkers();

    void AbandonMinedOutBases();

    std::unordered_map<std::shared_ptr<Expansion>, Units> m_expansionWorkers; // Map of Expansion -> Worker
};
