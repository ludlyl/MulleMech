// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Plugin.h"
#include "core/Units.h"
#include "plugins/micro/MicroPlugin.h"

struct ForceCommander : Plugin {
    ForceCommander();

    void OnStep(Builder*) final;

    void OnUnitCreated(Unit* unit_) final;

    void AttackEnemiesCloseToBase();

private:
    void UpdateOffensiveUnits();

    void RemoveDeadUnits();

    float m_attack_limit;
    Units m_units;
    Units m_offensiveUnits;

    bool m_inCombat;

    static constexpr float SearchEnemyRadius = 20.0f;
};
