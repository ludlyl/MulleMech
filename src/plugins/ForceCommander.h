// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Plugin.h"
#include "OffenseSquad.h"
#include "DefenseSquad.h"
#include "core/Units.h"
#include "plugins/micro/MicroPlugin.h"

struct ForceCommander : Plugin {
    ForceCommander();

    void OnStep(Builder*) final;

    void OnUnitCreated(Unit* unit_) final;

    void OnUnitDestroyed(Unit* unit_, Builder*) final;

    void AttackEnemiesCloseToBase();

private:
    float m_attack_limit;
    OffenseSquad m_mainSquad;
    std::vector<DefenseSquad> m_defenseSquads; // only one supported, temporary code for testing

    static constexpr float SearchEnemyRadiusPadding = 10.0f;
};
