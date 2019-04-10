
#include "CombatCommander.h"
#include "BuildingPlacer.h"
#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include <sc2api/sc2_common.h>

CombatCommander::CombatCommander() :
    m_mainAttackTarget(gAPI->observer().GameInfo().enemy_start_locations.front()),
    m_changedPlayStyle(true)
{

}

void CombatCommander::OnStep(Builder*){
    for (auto& squad : m_defenseSquads)
        squad.OnStep();
    m_mainSquad.OnStep();

    PlayStyle newPlayStyle = gReasoner->GetPlayStyle();
    if(newPlayStyle != m_playStyle){
        m_changedPlayStyle = true;
        m_playStyle = newPlayStyle;
    }

    if (m_playStyle != PlayStyle::all_in)
        DefenseCheck();

    switch(m_playStyle) {
        case PlayStyle::normal : PlayNormal();
            break;
        case PlayStyle::all_in : PlayAllIn();
            break;
        case PlayStyle::offensive : PlayOffensive();
            break;
        case PlayStyle::defensive : PlayDefensive();
            break;
        case PlayStyle::very_defensive : PlayVeryDefensive();
            break;
        case PlayStyle::greedy : PlayGreedy();
            break;
        case PlayStyle::scout : PlayScout();
            break;

    }

    m_changedPlayStyle = false;
}

void CombatCommander::PlayNormal(){
    if (m_mainSquad.IsTaskFinished() && gAPI->observer().GetFoodUsed() >= AttackOnSupply)
        m_mainSquad.TakeOver(m_mainAttackTarget);
    Harass(4);
}

void CombatCommander::PlayAllIn(){
    if (m_mainSquad.IsTaskFinished() && m_mainSquad.Size() > 0) {
        // Abandon defense and harass, then attack with all we got
        for (auto& def : m_defenseSquads)
            m_mainSquad.Absorb(def);
        m_defenseSquads.clear();
        m_mainSquad.Absorb(m_harassSquad);
        m_mainSquad.TakeOver(m_mainAttackTarget);
    }
}

void CombatCommander::PlayOffensive(){ // TODO
    PlayNormal();
}

void CombatCommander::PlayDefensive(){ // TODO
    if(m_changedPlayStyle){
        m_mainSquad.AbortTakeOver();
        gAPI->action().MoveTo(m_mainSquad.GetUnits(), gAPI->observer().StartingLocation());
    }
}

void CombatCommander::PlayVeryDefensive(){ // TODO
    PlayDefensive();
}

void CombatCommander::PlayGreedy(){ // TODO
    PlayNormal();
}

void CombatCommander::PlayScout(){ // TODO
    PlayNormal();
}

void CombatCommander::Harass(int limit){
    if (m_harassSquad.IsTaskFinished() && m_harassSquad.Size() > limit) {
        m_harassSquad.TakeOver(gAPI->observer().GameInfo().enemy_start_locations.front());
    }
}

std::vector<Units> CombatCommander::GroupEnemiesInBase() {
    // Calculate a circle using all our buildings for search radius and then increase it a bit
    // TODO: Improve this, using a circle for our base might spread way further than our perimeter
    //       is on some maps where our bases don't end up in a pattern fitting well in a circle
    float searchRadius = gAPI->observer().GetUnits(IsBuilding(), sc2::Unit::Alliance::Self)
            .CalculateCircle().second + SearchEnemyRadiusPadding;
    Units enemyUnits = gAPI->observer().GetUnits(IsWithinDist(gAPI->observer().StartingLocation(),
            searchRadius), sc2::Unit::Alliance::Enemy);

    // Split them up into groups that are together
    std::vector<Units> enemyGroups;
    while (!enemyUnits.empty()) {
        Units newGroup;
        auto leader = enemyUnits.front();
        newGroup.push_back(leader);
        enemyUnits.erase(enemyUnits.begin());

        // Add units "grouped" with the selected "leader"
        for (auto itr = enemyUnits.begin(); itr != enemyUnits.end(); ) {
            if (sc2::Distance3D(leader->pos, (*itr)->pos) <= EnemyGroupingDistance) {
                newGroup.push_back(*itr);
                itr = enemyUnits.erase(itr);
            } else {
                ++itr;
            }
        }

        enemyGroups.emplace_back(std::move(newGroup));
    }

    return enemyGroups;
}

void CombatCommander::DefenseCheck() {
    for (auto itr = m_defenseSquads.begin(); itr != m_defenseSquads.end(); ) {
        if (itr->IsTaskFinished()) {
            gHistory.info(LogChannel::combat) << "Defense Squad task finished, re-merging " <<
                                              itr->GetUnits().size() << " units to main squad" << std::endl;
            m_mainSquad.Absorb(*itr);
            itr = m_defenseSquads.erase(itr);
        }
        else
            ++itr;
    }

    auto enemyGroups = GroupEnemiesInBase();
    if (enemyGroups.empty())
        return;

    for (auto& group : enemyGroups) {
        bool dealtWith = false;

        // Update enemy list if we have a squad dealing with them already
        for (auto& squad : m_defenseSquads) {
            if (squad.UpdateEnemies(group)) {
                dealtWith = true;
                break;
            }
        }

        // Need a new squad to deal with this enemy group?
        if (!dealtWith) {
            // TODO: Intelligent defender selection
            Units defenders;
            int steal = group.size() + 1;
            while (--steal >= 0 && StealUnitFromMainSquad(defenders))
                /* empty */;
            if (!defenders.empty())
                m_defenseSquads.emplace_back(std::move(defenders), std::move(group));
        }
    }

    // Add more defenders if necessary (TODO: Intelligent selection)
    for (auto& defSquad : m_defenseSquads) {
        int diff = static_cast<int>(defSquad.GetEnemies().size()) - static_cast<int>(defSquad.Size());
        while (--diff >= 0) {
            auto unit = m_mainSquad.GetUnits().GetRandomUnit();
            if (!unit)
                return; // No more units
            defSquad.AddUnit(unit);
            m_mainSquad.RemoveUnit(unit);
        }
    }
}

void CombatCommander::OnUnitCreated(Unit* unit_){
    if (!IsCombatUnit()(*unit_))
        return;
    if(unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_REAPER){
        m_harassSquad.AddUnit(unit_);
    } else {
        m_mainSquad.AddUnit(unit_);
        if (m_mainSquad.IsTaskFinished())
            gAPI->action().MoveTo(m_mainSquad.GetUnits(), GetArmyIdlePosition(), true);
    }
}

void CombatCommander::OnUnitDestroyed(Unit* unit_, Builder*) {
    m_mainSquad.RemoveUnit(unit_);
    if (!m_defenseSquads.empty())
        for (auto& squad : m_defenseSquads)
            squad.RemoveUnit(unit_);
}

bool CombatCommander::StealUnitFromMainSquad(Units& defenders) {
    if (!m_mainSquad.GetUnits().empty()) {
        auto unit = m_mainSquad.GetUnits().GetRandomUnit();
        defenders.push_back(unit);
        m_mainSquad.RemoveUnit(unit);
        return true;
    }
    return false;
}

sc2::Point3D CombatCommander::GetArmyIdlePosition() const {
    // TODO: Calculate a more sensible position of where to keep our army

    auto expansions = gHub->GetOurExpansions();
    if (expansions.empty())
        return gAPI->observer().StartingLocation();

    auto town_hall = expansions.back()->town_hall_location;
    auto direction_vector = town_hall - BuildingPlacer::GetCenterBehindMinerals(town_hall);
    sc2::Normalize3D(direction_vector);

    return town_hall + direction_vector * IdleDistance;
}
