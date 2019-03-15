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
    
    int const number_of_barracks = gAPI->observer().GetUnits(IsBarracks(), sc2::Unit::Alliance::Self)().size();
    int const number_of_factories = gAPI->observer().GetUnits(IsFactory(), sc2::Unit::Alliance::Self)().size();
    int const number_of_starports = gAPI->observer().GetUnits(IsStarport(), sc2::Unit::Alliance::Self)().size();
    int const number_of_commandCenters = gAPI->observer().GetUnits(IsCommandCenter(), sc2::Unit::Alliance::Self)().size();
    int const number_of_refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self)().size();

    float mineral_income = gAPI->observer().GetMineralIncome();
    float vespene_income = gAPI->observer().GetVespeneIncome();
    std::pair<float, float> consumption = CurrentConsumption();
    float mineral_consumption = consumption.first;
    float vespene_consumption = consumption.second;

    float mineral_overproduction = mineral_income - mineral_consumption;
    float vespene_overproduction = vespene_income - vespene_consumption;

    //TODO start planning on what we want to spend our overproduction on based on current army compersition
    //TODO or expand our base based on enum from higher-order plugin.

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

std::pair<float, float> Governor::CurrentConsumption() {
    //TODO add buildings that are being built and that are in building queue to calculations
    auto barracks = gAPI->observer().GetUnits(IsBarracks(), sc2::Unit::Alliance::Self);
    auto factories = gAPI->observer().GetUnits(IsFactory(), sc2::Unit::Alliance::Self);
    auto starports = gAPI->observer().GetUnits(IsStarport(), sc2::Unit::Alliance::Self);
    float mineral_consumption = 0; // Minerals/min
    float vespene_consumption = 0; // Vespene/min

    for (const auto& i : barracks()) {
        //assumed zero production incase of mechbuild
        if (i->add_on_tag == 0) {
            continue;
        }
        auto addOnAsUnit = gAPI->observer().GetUnit(i->add_on_tag);
        auto type = addOnAsUnit->unit_type.ToType();

        switch (type) {
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSREACTOR:
            break;
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB:
            break;
        default:
            break;
        }
    }

    for (const auto& i : factories()) {
        //TODO if factory doesnt have addon -> check what addon it's building assuming it's building one
        if (i->add_on_tag == 0) {
            continue;
        }

        auto addOnAsUnit = gAPI->observer().GetUnit(i->add_on_tag);
        auto type = addOnAsUnit->unit_type.ToType();

        switch (type) {
        case sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR:
            //assuming hellion prodcution
            mineral_consumption += 2.0 * ( 100.0 / (21.0 / 60.0));
            break;
        case sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB:
            // assuming tank production
            mineral_consumption += (150.0 / (32.0 / 60.0));
            vespene_consumption += 125.0 / (32.0 / 60.0);
            break;
        default:
            break;
        }

    }

    // TODO Starport wont produce units continiusly, add a flag to know when it is and when it isn't.
    for (const auto& i : starports()) {
        //TODO if starport doesnt have addon -> check what addon it's building assuming it's building one
        if (i->add_on_tag == 0) {
            continue;
        }

        auto addOnAsUnit = gAPI->observer().GetUnit(i->add_on_tag);
        auto type = addOnAsUnit->unit_type.ToType();

        switch (type) {
        case sc2::UNIT_TYPEID::TERRAN_STARPORTREACTOR:
            //assuming viking production
            mineral_consumption += 2.0 * 150.0 / (30.0 / 60.0);
            vespene_consumption += 2.0 * 75.0 / (30.0 / 60.0);
            break;
        case sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB:
            //assuming banshee production
            mineral_consumption += 150.0 / (30.0 / 60.0);
            vespene_consumption += 100.0 / (30.0 / 60.0);
            break;
        default:
            break;
        }
    }

    std::pair<float, float> total_consumption;
    total_consumption.first = mineral_consumption;
    total_consumption.second = vespene_consumption;
    return total_consumption;
}
