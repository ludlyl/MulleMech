// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "ForceCommander.h"
#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_map_info.h>

#include <algorithm>

ForceCommander::ForceCommander() : m_attack_limit(8) {
}

void ForceCommander::AttackEnemiesCloseToBase() {
    if (!m_defenseSquads.empty() || m_mainSquad.GetUnits().empty())
        return;

    // Calculate a circle using all our buildings for search radius and then increase it a bit
    float searchRadius = gAPI->observer().GetUnits(IsBuilding(),
        sc2::Unit::Alliance::Self).CalculateCircle().second + SearchEnemyRadiusPadding;
    auto enemyUnits = gAPI->observer().GetUnits(IsWithinDist(
        gAPI->observer().StartingLocation(), searchRadius), sc2::Unit::Alliance::Enemy);
    if (enemyUnits.empty())
        return;

    Units defenseUnits;
    int steal = static_cast<int>(enemyUnits.size()) + 1;
    while (--steal >= 0 && !m_mainSquad.GetUnits().empty()) {
        auto unit = m_mainSquad.GetUnits().GetRandomUnit();
        defenseUnits.push_back(unit);
        m_mainSquad.RemoveUnit(unit);
    }

    m_defenseSquads.push_back(DefenseSquad(std::move(defenseUnits), std::move(enemyUnits)));
}

void ForceCommander::OnStep(Builder*) {
    // TODO: If the enemies retreat out of vision our defense squad will
    //       remain as they never disposed the targets we gave them
    for (auto itr = m_defenseSquads.begin(); itr != m_defenseSquads.end(); ) {
        itr->OnStep();
        if (itr->IsTaskFinished()) {
            gHistory.info(LogChannel::combat) << "Defense Squad task finished, re-merging " <<
                itr->GetUnits().size() << " units to main squad" << std::endl;
            m_mainSquad.Absorb(*itr);
            itr = m_defenseSquads.erase(itr);
        }
        else
            ++itr;
    }

    AttackEnemiesCloseToBase();

    m_mainSquad.OnStep();

    if (m_mainSquad.GetUnits().size() < m_attack_limit)
        return;

    if (!m_mainSquad.IsTaskFinished())
        return;

    auto targets = gAPI->observer().GameInfo().enemy_start_locations;
    m_mainSquad.TakeOver(targets.front());

    m_attack_limit = std::min<float>(m_attack_limit * 1.5f, 170.0f);
}

void ForceCommander::OnUnitCreated(Unit* unit_) {
    if (!IsCombatUnit()(*unit_))
        return;

    m_mainSquad.AddUnit(unit_);
}

void ForceCommander::OnUnitDestroyed(Unit* unit_, Builder*) {
    m_mainSquad.RemoveUnit(unit_);
    if (!m_defenseSquads.empty())
        for (auto& squad : m_defenseSquads)
            squad.RemoveUnit(unit_);
}
