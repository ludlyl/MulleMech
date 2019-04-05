
#include "CombatCommander.h"
#include "core/API.h"
#include "Hub.h"
#include <core/Helpers.h>
#include <Historican.h>
//#include <sc2api/sc2_map_info.h>

CombatCommander::CombatCommander() :
    m_mainAttackTarget(gAPI->observer().GameInfo().enemy_start_locations.front()),
    m_attack_limit(10),
    m_changedPlayStyle(true)
{

}

void CombatCommander::OnStep(Builder*){
    DefenseCheck();

    for(DefenseSquad d_squad : m_defenseSquads){
        d_squad.OnStep();
    }
    m_mainSquad.OnStep();

    PlayStyle newPlayStyle = gReasoner->GetPlayStyle();
    if(newPlayStyle != m_playStyle){
        m_changedPlayStyle = true;
        m_playStyle = newPlayStyle;
    }

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
    if(m_mainSquad.HasTask()){
        if(m_mainSquad.IsTaskFinished()){
            GiveMainSquadNewTask();
        }
    } else if(m_mainSquad.Size()> m_attack_limit){
        m_mainSquad.TakeOver(m_mainAttackTarget);
    }
    Harass(4);
}

void CombatCommander::PlayAllIn(){
    if(!m_mainSquad.HasTask() && m_mainSquad.Size() > 0){
        m_mainSquad.TakeOver(m_mainAttackTarget);
    } else {
        if(m_mainSquad.IsTaskFinished()){
            GiveMainSquadNewTask();
        }
    }
    Harass(0);
}

void CombatCommander::PlayOffensive(){ // TODO
    PlayAllIn();
}

void CombatCommander::PlayDefensive(){ // TODO
    if(m_changedPlayStyle){
        if(m_mainSquad.HasTask()){
            m_mainSquad.AbortTakeOver();
            //gAPI->MoveTo(m_mainSquad.GetUnits(), gAPI->observer().StartingLocation());
        }
    }
}

void CombatCommander::PlayVeryDefensive(){ // TODO
    PlayNormal();
}

void CombatCommander::PlayGreedy(){ // TODO
    PlayNormal();
}

void CombatCommander::PlayScout(){ // TODO
    PlayNormal();
}

//TODO
void CombatCommander::GiveMainSquadNewTask(){
        // Called when mainSquad has finished its task
}

void CombatCommander::Harass(int limit){
    if(!m_harassSquad.HasTask() && m_harassSquad.Size() > limit){
        m_harassSquad.TakeOver(gAPI->observer().GameInfo().enemy_start_locations.front());
    }
}

Units CombatCommander::LookForEnemiesInBase(){
    // Calculate a circle using all our buildings for search radius and then increase it a bit
    float searchRadius = gAPI->observer().GetUnits(IsBuilding(), sc2::Unit::Alliance::Self)
            .CalculateCircle().second + SearchEnemyRadiusPadding;
    Units enemyUnits = gAPI->observer().GetUnits(IsWithinDist(gAPI->observer().StartingLocation(),
            searchRadius), sc2::Unit::Alliance::Enemy);
    return enemyUnits;
}

void CombatCommander::DefenseCheck(){
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

    Units enemyUnits = LookForEnemiesInBase();
    if(enemyUnits.empty()){
        return;
    }

    if(!m_defenseSquads.empty()){
        int diff = enemyUnits.size() - m_defenseSquads.front().Size();
        if (diff < 0) return;
        while(--diff >= 0 && !m_mainSquad.GetUnits().empty()){
            auto unit = m_mainSquad.GetUnits().GetRandomUnit();
            m_defenseSquads.front().AddUnit(unit);
            m_mainSquad.RemoveUnit(unit);
        }
        m_defenseSquads.front().SetEnemies(enemyUnits);
    } else {
        Units defenseUnits;
        int steal = static_cast<int>(enemyUnits.size()) + 1;
        while (--steal >= 0 && !m_mainSquad.GetUnits().empty()) {
            auto unit = m_mainSquad.GetUnits().GetRandomUnit();
            defenseUnits.push_back(unit);
            m_mainSquad.RemoveUnit(unit);
        }
    }
}

void CombatCommander::OnUnitCreated(Unit* unit_){
    if (!IsCombatUnit()(*unit_))
        return;
    if(unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_REAPER){
        m_harassSquad.AddUnit(unit_);
    }
    m_mainSquad.AddUnit(unit_);
}

void CombatCommander::OnUnitDestroyed(Unit* unit_, Builder*) {
    m_mainSquad.RemoveUnit(unit_);
    if (!m_defenseSquads.empty())
        for (auto& squad : m_defenseSquads)
            squad.RemoveUnit(unit_);
}
