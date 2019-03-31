// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Governor.h"
#include "core/Helpers.h"
#include "core/Converter.h"
#include "Historican.h"
#include "Hub.h"

#include <sc2api/sc2_agent.h>

//Placeholder, might wanna move this in future
#define FRAMES_PER_SECOND 21.4f

void Governor::OnGameStart(Builder* builder_) {
    // Initial build order
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
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
            break;
    }
}

void Governor::OnStep(Builder* builder_) {
    int minerals = gAPI->observer().GetMinerals();

    int hellion_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_HELLION).mineral_cost;
    float hellion_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_HELLION).build_time;

    int tank_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SIEGETANK).mineral_cost;
    int tank_vespene = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SIEGETANK).vespene_cost;
    float tank_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SIEGETANK).build_time;

    if (minerals < 50)
       return;
    //TODO add priority flag for factory production
    //TODO create exeception handler for planner_queue
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

    // Note: Returns Minerals/Min
    float mineral_income = gAPI->observer().GetMineralIncomeRate();
    float vespene_income = gAPI->observer().GetVespeneIncomeRate();
    std::pair<float, float> consumption = CurrentConsumption(builder_); // Note: Returns Mineral/frame, Vespene/frame

    float mineral_consumption = consumption.first;
    float vespene_consumption = consumption.second;

    //Converting from Mineral/frames to Mineral/min
    mineral_consumption = mineral_consumption * FRAMES_PER_SECOND * 60.f;
    vespene_consumption = vespene_consumption * FRAMES_PER_SECOND * 60.f;

    //Values here are in Minerals/min
    float mineral_overproduction = mineral_income - mineral_consumption;
    float vespene_overproduction = vespene_income - vespene_consumption;

    gHistory.info() << mineral_consumption << std::endl;

    if (mineral_overproduction < 0)
        return;

    if (vespene_overproduction < 0) {
        // In this case we have minerals but not vespene -> produce hellions

        //cost of factory with reactor producing  
        if (mineral_overproduction < (FRAMES_PER_SECOND * 60.f * 2.f * hellion_mineral / hellion_build_time)) 
            return;

        //TODO Confirm this value (2)
        //We never want more than 2 factories producing hellions
        if (CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR) > 1)
            return;

        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR);
    }

    if (mineral_overproduction > (FRAMES_PER_SECOND * 60.f * tank_mineral / tank_build_time) &&
        vespene_overproduction > (FRAMES_PER_SECOND * 60.f * tank_vespene / tank_build_time)) {

        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
    }
    //TODO expand our base based on enum from higher-order plugin.
}

int Governor::CountTotalStructures(Builder* builder_, sc2::UNIT_TYPEID type) {
    int total_structures = (int)builder_->CountScheduledStructures(type);

    for (const auto i : m_planner_queue) {
        if (i == type)
            total_structures++;
    }

    total_structures += (int)gAPI->observer().GetUnits(IsUnit(type, true),
        sc2::Unit::Alliance::Self).size();

    return total_structures;
}

void Governor::OnUnitIdle(Unit *unit_, Builder *builder_) {
    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
            break;
        case sc2::UNIT_TYPEID::TERRAN_FACTORY:
            //TODO sometimes we might want to produce cyclons
            if (HasAddon(sc2::UNIT_TYPEID::TERRAN_TECHLAB)(*unit_)) {
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_SIEGETANK, false, unit_);
                gHistory.info() << "Schedule siegetank training" << std::endl;
                return;
            }
            else if (HasAddon(sc2::UNIT_TYPEID::TERRAN_REACTOR)(*unit_)) {
                //TODO We don't always want to schedule 2 units here...
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_HELLION, false, unit_);
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_HELLION, false, unit_);
                gHistory.info() << "Schedule double hellion training" << std::endl;
                return;
            } else {
                // Naked
            }
            break;
         case sc2::UNIT_TYPEID::TERRAN_STARPORT:
            //TODO decide how we want to manage to production here
            break;
        default:
            break;
    }
}

void Governor::OnBuildingConstructionComplete(Unit* unit_) {
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
        gHistory.debug() << "Lowering new Supply Depot" << std::endl;
        gAPI->action().LowerDepot(unit_);
    }
}

std::pair<float, float> Governor::CurrentConsumption(Builder* builder_) {
    float mineral_consumption = 0; // Minerals/frame
    float vespene_consumption = 0; // Vespene/frame


    int hellion_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_HELLION).mineral_cost;
    float hellion_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_HELLION).build_time;

    int tank_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SIEGETANK).mineral_cost;
    int tank_vespene = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SIEGETANK).vespene_cost;
    float tank_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SIEGETANK).build_time;

    int viking_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER).mineral_cost;
    int viking_vespene = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER).vespene_cost;
    float viking_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER).build_time;

    int banshee_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_BANSHEE).mineral_cost;
    int banshee_vespene = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_BANSHEE).vespene_cost;
    float banshee_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_BANSHEE).build_time;


    //Since we will never have a production building without an addon it's safe to calculate cost based on addons
    mineral_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR) *
        2.f * hellion_mineral / hellion_build_time;

    mineral_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB) *
         tank_mineral / tank_build_time;
    vespene_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB) *
        tank_vespene / tank_build_time;

    mineral_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_STARPORTREACTOR) *
        2.f * viking_mineral / viking_build_time;
    vespene_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_STARPORTREACTOR) *
        2.f * viking_vespene / viking_build_time;

    mineral_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB) *
        banshee_mineral / banshee_build_time;
    vespene_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB) *
        banshee_vespene / banshee_build_time;

    std::pair<float, float> total_consumption;
    total_consumption.first = mineral_consumption;
    total_consumption.second = vespene_consumption;
    return total_consumption;
}
