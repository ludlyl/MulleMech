// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "../Historican.h"
#include "ForceCommander.h"
#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_map_info.h>

#include <algorithm>
#include <Hub.h>


ForceCommander::ForceCommander(): m_attack_limit(16) {
}

void ForceCommander::AttackEnemiesCloseToBase(){
    auto enemyUnits = gAPI->observer().GetUnits(sc2::Unit::Alliance::Enemy);
    sc2::Point3D baseLocation3D = gAPI->observer().StartingLocation();
    sc2::Point2D baseLocation2D = sc2::Point2D(baseLocation3D.x, baseLocation3D.y);
    const sc2::Unit* closestEnemyUnit = enemyUnits.GetClosestUnit(baseLocation2D);
    if(closestEnemyUnit == nullptr) return;
    sc2::Point3D enemyPos3D = closestEnemyUnit->pos;
    sc2::Point2D enemyPos2D = sc2::Point2D(enemyPos3D.x, enemyPos3D.y);
    double lengthToEnemy = sqrt(std::pow(baseLocation3D.x-enemyPos3D.x, 2) + std::pow(baseLocation3D.y-enemyPos3D.y, 2));
    int limit = 50;
    if(lengthToEnemy < limit){
        gAPI->action().Attack(m_units, enemyPos2D);
    }
}

void ForceCommander::OnStep(Builder*) {
    // Clean up dead bodies.
    auto it = std::remove_if(m_units.begin(), m_units.end(),
        [](const sc2::Unit* unit_) {
            return !unit_->is_alive;
        });

    m_units.erase(it, m_units.end());

    AttackEnemiesCloseToBase();

    if (m_units.size() < m_attack_limit)
        return;

    auto targets = gAPI->observer().GameInfo().enemy_start_locations;
    gAPI->action().Attack(m_units, targets.front());

    m_units.clear();
    m_attack_limit = std::min<float>(m_attack_limit * 1.5f, 170.0f);
}

void ForceCommander::OnUnitCreated(const sc2::Unit* unit_) {
    if (!IsCombatUnit()(*unit_))
        return;

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
        " added to attack group" << std::endl;

    m_units.push_back(unit_);
}
