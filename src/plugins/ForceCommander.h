// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Plugin.h"
#include "plugins/micro/MicroPlugin.h"

struct ForceCommander : Plugin {
    ForceCommander();

    void OnStep(Builder*) final;

    void OnUnitCreated(const sc2::Unit* unit_) final;

    void AttackEnemiesCloseToBase();

private:
    float m_attack_limit;

    sc2::Units m_units;

    // TODO: Maybe we should have a Unit wrapper (like we do Units) so we could extend it with stuff like a micro plugin
    sc2::Units m_offensiveUnits;
    std::unordered_map<const sc2::Unit*, std::shared_ptr<MicroPlugin>> m_activePlugins;

    bool m_inCombat;

    void UpdateOffensiveUnits();

    void RemoveDeadUnits();
};
