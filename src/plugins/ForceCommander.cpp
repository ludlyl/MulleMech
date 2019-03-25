// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "../Historican.h"
#include "ForceCommander.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_map_info.h>

#include <algorithm>

ForceCommander::ForceCommander() : m_attack_limit(16), m_inCombat(false) {
}

void ForceCommander::AttackEnemiesCloseToBase() {
    auto enemyUnits = gAPI->observer().GetUnits(sc2::Unit::Alliance::Enemy);
    sc2::Point3D baseLocation3D = gAPI->observer().StartingLocation();
    sc2::Point2D baseLocation2D = sc2::Point2D(baseLocation3D.x, baseLocation3D.y);
    auto closestEnemyUnit = enemyUnits.GetClosestUnit(baseLocation2D);
    if (!closestEnemyUnit)
        return;
    sc2::Point3D enemyPos3D = closestEnemyUnit->pos;
    sc2::Point2D enemyPos2D = sc2::Point2D(enemyPos3D.x, enemyPos3D.y);
    double lengthToEnemy = sqrt(std::pow(baseLocation3D.x-enemyPos3D.x, 2) + std::pow(baseLocation3D.y-enemyPos3D.y, 2));
    int limit = 50;
    if (lengthToEnemy < limit) {
        gAPI->action().Attack(m_units, enemyPos2D);
    }
}

void ForceCommander::OnStep(Builder*) {
    RemoveDeadUnits();

    AttackEnemiesCloseToBase();

    UpdateOffensiveUnits();

    if (m_units.size() < m_attack_limit)
        return;

    auto targets = gAPI->observer().GameInfo().enemy_start_locations;
    gAPI->action().MoveTo(m_units, targets.front());

    for (auto& unit : m_units) {
        m_offensiveUnits.push_back(unit);
    }
    m_units.clear();

    m_attack_limit = std::min<float>(m_attack_limit * 1.5f, 170.0f);
}

void ForceCommander::OnUnitCreated(Unit* unit_) {
    if (!IsCombatUnit()(*unit_))
        return;

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
        " added to attack group" << std::endl;

    m_units.push_back(unit_);
}

void ForceCommander::UpdateOffensiveUnits() {
    if (m_offensiveUnits.empty())
        return;

    auto nearbyEnemies = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
        {IsWithinDist(m_offensiveUnits[0]->pos, SearchEnemyRadius), IsCombatUnit()}), sc2::Unit::Alliance::Enemy);

    // If all enemies are dead => Continue moving
    if (nearbyEnemies.empty() && m_inCombat) {
        auto pos = gAPI->observer().GameInfo().enemy_start_locations.front();
        for (auto& unit : m_offensiveUnits) {
            gAPI->action().MoveTo(unit, pos);
            unit->Micro()->OnCombatOver(unit);
        }
        m_inCombat = false;
    // If we still have enemies => Update micro plugins
    } else if (!nearbyEnemies.empty()) {
        m_inCombat = true;
        for (auto& unit : m_offensiveUnits) {
            unit->Micro()->OnCombatFrame(unit, nearbyEnemies);
        }
    }
}

void ForceCommander::RemoveDeadUnits() {
    for (auto itr = m_units.begin(); itr != m_units.end(); ) {
        if ((*itr)->is_alive)
            ++itr;
        else
            itr = m_units.erase(itr);
    }

    for (auto itr = m_offensiveUnits.begin(); itr != m_offensiveUnits.end(); ) {
        if ((*itr)->is_alive)
            ++itr;
        else
            itr = m_offensiveUnits.erase(itr);
    }
}
