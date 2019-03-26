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
}

void Governor::OnStep(Builder* builder_) {
    int minerals = gAPI->observer().GetMinerals();

    int hellion_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_SIEGETANK).mineral_cost;
    float hellion_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_HELLION).build_time;

    if (minerals < 50)
       return;
    //TODO add priority flag for factory production
    //TODO create exception handler for planner_queue
    //TODO Army composition for military production
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

    float mineral_income = gAPI->observer().GetMineralIncomeRate();
    float vespene_income = gAPI->observer().GetVespeneIncomeRate();
    std::pair<float, float> consumption = CurrentConsumption();
    float mineral_consumption = consumption.first;
    float vespene_consumption = consumption.second;

    float mineral_overproduction = mineral_income - mineral_consumption;
    float vespene_overproduction = vespene_income - vespene_consumption;

    if (mineral_overproduction < 0)
        return;

    if (vespene_overproduction < 0) {
        // In this case we have minerals but not vespene -> produce hellions

        if (mineral_overproduction < (2.f * hellion_mineral / (hellion_build_time / 60.f))) //cost of factory with reactor producing
            return;

    }
    //TODO start planning on what we want to spend our overproduction on based on current army composition
    //TODO or expand our base based on enum from higher-order plugin.
}

void Governor::OnUnitIdle(Unit *unit_, Builder *builder_) {
    sc2::UNIT_TYPEID type;
    Unit* addOnAsUnit;

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
            if (unit_->add_on_tag != 0) {
                addOnAsUnit = gAPI->observer().GetUnit(unit_->add_on_tag);
                type = addOnAsUnit->unit_type;
                if (type == sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB) {
                    builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_MARAUDER, false, unit_);
                    gHistory.info() << "Schedule Marauder training" << std::endl;
                    return;
                }
            }
            break;
        case sc2::UNIT_TYPEID::TERRAN_FACTORY:
            //TODO sometimes we might want to produce cyclons
            if (unit_->add_on_tag == 0)
                return;
            addOnAsUnit = gAPI->observer().GetUnit(unit_->add_on_tag);
            type = addOnAsUnit->unit_type;
            if (type == sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB) {
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_SIEGETANK, false, unit_);
                gHistory.info() << "Schedule siegetank training" << std::endl;
                return;
            }
            if (type == sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR) {
                //TODO fix so that this will always build 2 hellions at all times.
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_HELLION, false, unit_);
                gHistory.info() << "Schedule Hellion training" << std::endl;
                return;
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


//TODO this function should take a State as input. The state can be the current state or a future state
std::pair<float, float> Governor::CurrentConsumption() {
    //TODO add buildings that are being built and that are in building queue to calculations
    auto barracks = gAPI->observer().GetUnits(IsUnit(sc2::UNIT_TYPEID::TERRAN_BARRACKS), sc2::Unit::Alliance::Self);
    auto factories = gAPI->observer().GetUnits(IsUnit(sc2::UNIT_TYPEID::TERRAN_FACTORY), sc2::Unit::Alliance::Self);
    auto starports = gAPI->observer().GetUnits(IsUnit(sc2::UNIT_TYPEID::TERRAN_STARPORT), sc2::Unit::Alliance::Self);
    float mineral_consumption = 0; // Minerals/min
    float vespene_consumption = 0; // Vespene/min
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

    for (const auto& i : barracks) {
        //assumed zero production incase of mechbuild
        if (i->add_on_tag == 0) {
            continue;
        }
        auto type = i->unit_type.ToType();

        switch (type) {
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSREACTOR:
            break;
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB:
            break;
        default:
            break;
        }
    }

    for (const auto& i : factories) {
        //TODO if factory doesnt have addon -> check what addon it's building assuming it's building one
        if (i->add_on_tag == 0) {
            continue;
        }

        auto type = i->unit_type.ToType();

        switch (type) {
        case sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR:
            //assuming hellion production

            mineral_consumption += 2.f * ( hellion_mineral / (hellion_build_time / 60.f));
            break;
        case sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB:
            // assuming tank production

            mineral_consumption += tank_mineral / (tank_build_time / 60.f);
            vespene_consumption += tank_vespene / (tank_build_time / 60.f);
            break;
        default:
            break;
        }
    }

    // TODO Starport wont produce units continuously, add a flag to know when it is and when it isn't.
    for (const auto& i : starports) {
        //TODO if starport doesnt have addon -> check what addon it's building assuming it's building one
        if (i->add_on_tag == 0) {
            continue;
        }

        auto type = i->unit_type.ToType();

        switch (type) {
        case sc2::UNIT_TYPEID::TERRAN_STARPORTREACTOR:
            //assuming viking production

            mineral_consumption += 2.f * viking_mineral / (viking_build_time / 60.f);
            vespene_consumption += 2.f * viking_vespene / (viking_build_time / 60.f);
            break;
        case sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB:
            //assuming banshee production

            mineral_consumption += banshee_mineral / (banshee_build_time / 60.f);
            vespene_consumption += banshee_vespene / (banshee_build_time / 60.f);
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
