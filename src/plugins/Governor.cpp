// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "core/Helpers.h"
#include "core/Converter.h"
#include "../Historican.h"
#include "../Hub.h"
#include "Governor.h"

#include <sc2api/sc2_agent.h>

void Governor::OnGameStart(Builder* builder_) {
    // Initial build order
    gHistory.info() << "Started game as Terran" << std::endl;
    builder_->ScheduleConstruction(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT);

    enum Strategies {mech, bio, bunkerRush};
    Strategies strategy = mech;

    switch (strategy) {
        case mech:
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_BARRACKS);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ARMORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
            break;
    }
    return;
}

void Governor::OnStep(Builder* builder_) {
    int minerals = gAPI->observer().GetMinerals();

    if (minerals < 50)
       return;
    //TODO add priority flag for factory production
    //TODO create exeception handler for planner_queue
    //TODO Army compesition for military production
    //TODO Get current army
    //TODO compare military strength of us and enemy
    auto it = m_planner_queue.begin();
    int planned_cost = 0;
    while (it != m_planner_queue.end()) {
        planned_cost += gAPI->observer().GetUnitTypeData(m_planner_queue.front()).mineral_cost;
        if (minerals < planned_cost)
            return;
        minerals -= planned_cost;
        builder_->ScheduleConstruction(m_planner_queue.front());
        it = m_planner_queue.erase(it);
    }
    float mineral_income = gAPI->observer().GetMineralIncome();
    float vespene_income = gAPI->observer().GetVespeneIncome();
    
    int const number_of_barracks = gAPI->observer().GetUnits(IsBarracks(), sc2::Unit::Alliance::Self)().size();
    int const number_of_factories = gAPI->observer().GetUnits(IsFactory(), sc2::Unit::Alliance::Self)().size();
    int const number_of_starports = gAPI->observer().GetUnits(IsStarport(), sc2::Unit::Alliance::Self)().size();
    int const number_of_commandCenters = gAPI->observer().GetUnits(IsCommandCenter(), sc2::Unit::Alliance::Self)().size();
    int const number_of_refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self)().size(); 

}

void Governor::OnUnitIdle(const sc2::Unit *unit_, Builder *builder_) {
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_BARRACKS) {
        if (unit_->add_on_tag != 0) {
            auto addOnAsUnit = gAPI->observer().GetUnit(unit_->add_on_tag);
            auto type = addOnAsUnit->unit_type.ToType();
            if (type == sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB) {
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_MARAUDER, false, unit_);
                gHistory.info() << "Schedule Marauder training" << std::endl;
                return;
            }
        }
    }

    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_GATEWAY ||
            unit_->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_WARPGATE) {
        builder_->ScheduleTraining(sc2::UNIT_TYPEID::PROTOSS_ZEALOT, false, unit_);
        gHistory.info() << "Schedule Zealot training" << std::endl;
        return;
    }
}

void Governor::OnBuildingConstructionComplete(const sc2::Unit* unit_) {
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
        gHistory.debug() << "Lowering new Supply Depot" << std::endl;
        gAPI->action().LowerDepot(*unit_);
    }
}

void Governor::CurrentConsumption() {
    auto command_centers = gAPI->observer().GetUnits(IsCommandCenter(), sc2::Unit::Alliance::Self);
    float total_production = 0;
    for (const auto& i : command_centers()) {
        
    }
}
