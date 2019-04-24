// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Governor.h"
#include "core/Helpers.h"
#include "core/Converter.h"
#include "Historican.h"
#include "Hub.h"
#include "Reasoner.h"

#include <sc2api/sc2_agent.h>


void Governor::OnGameStart(Builder* builder_) {
    // Initial build order
    builder_->ScheduleSequentialConstruction(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT);

    enum Strategies {mech, bio, bunkerRush};
    Strategies strategy = mech;

    switch (strategy) {
        case mech:
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_BARRACKS);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ENGINEERINGBAY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ARMORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_ARMORY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_STARPORT);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB);

            // Armory Upgrades
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL1);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL2);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL3);

            //builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL1); TODO fix so that these work in
            //builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL2);
            //builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL3);

            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANSHIPWEAPONSLEVEL1);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANSHIPWEAPONSLEVEL2);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANSHIPWEAPONSLEVEL3);

            // Tech lab upgrades
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::DRILLCLAWS);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::SMARTSERVOS);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::RAVENCORVIDREACTOR);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::MEDIVACINCREASESPEEDBOOST);
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::HIGHCAPACITYBARRELS);

            // Fusion Core upgrades
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::BATTLECRUISERENABLESPECIALIZATIONS);

            //Engineering Bay Upgrades (will ignore infantry upgrades since)
            //builder_->ScheduleUpgrade(sc2::UPGRADE_ID::TERRANBUILDINGARMOR); //TODO fix so that this works
            builder_->ScheduleUpgrade(sc2::UPGRADE_ID::HISECAUTOTRACKING); 

            //TODO add ghost acadamy if needed.
            break;
        default:
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

    PlayStyle playstyle = gReasoner->GetPlayStyle();
    float greed_modifier = 1.f;

    //TODO, fill in the other ones as well
    switch (playstyle) {
    case PlayStyle::all_in:
    case PlayStyle::defensive:
    case PlayStyle::very_defensive:
    case PlayStyle::offensive:
    case PlayStyle::scout:
    case PlayStyle::normal:
        greed_modifier = 1.f;
        break;

    case PlayStyle::greedy:
        // This modifier is right now highly questionable.
        greed_modifier = 2.f;
        // resort the list
        PrioritizeCommandCenter();
        break;
    }

    if (minerals < 50)
       return;
    //TODO add priority flag for factory production
    //TODO create exception handler for planner_queue
    auto it = m_planner_queue.begin();
    int planned_cost = 0;
    while (it != m_planner_queue.end()) {
        planned_cost += gAPI->observer().GetUnitTypeData(m_planner_queue.front()).mineral_cost;
        if (minerals < planned_cost)
            return;
        minerals -= planned_cost;
        builder_->ScheduleConstructionInRecommendedQueue(m_planner_queue.front());
        it = m_planner_queue.erase(it);
    }

    // Note: Returns Minerals/Min
    float mineral_income = gAPI->observer().GetMineralIncomeRate();
    float vespene_income = gAPI->observer().GetVespeneIncomeRate();
    std::pair<float, float> consumption = CurrentConsumption(builder_); // Note: Returns Mineral/frame, Vespene/frame

    float mineral_consumption = consumption.first;
    float vespene_consumption = consumption.second;

    //Converting from Mineral/frames to Mineral/min
    mineral_consumption = mineral_consumption * StepsPerSecond * 60.f;
    vespene_consumption = vespene_consumption * StepsPerSecond * 60.f;

    //Values here are in Minerals/min
    float mineral_overproduction = mineral_income - mineral_consumption;
    float vespene_overproduction = vespene_income - vespene_consumption;

    if (mineral_overproduction > (StepsPerSecond * 60.f * tank_mineral / tank_build_time) &&
        vespene_overproduction > (StepsPerSecond * 60.f * tank_vespene / tank_build_time)) {
        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);
    //TODO Confirm this value (2)
    //We never want more than 2 factories producing hellions
    } else if (mineral_overproduction > (StepsPerSecond * 60.f * 2.f * hellion_mineral / hellion_build_time) &&
               CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR) < 2) {
        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORY);
        m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR);
    }

    //TODO expand our base based on enum from higher-order plugin.
    auto command_centers = gAPI->observer().GetUnits(IsTownHall(), sc2::Unit::Alliance::Self);
    auto refineries = gAPI->observer().GetUnits(IsRefinery(), sc2::Unit::Alliance::Self);
    auto num_workers = static_cast<int>(gAPI->observer().GetUnits(IsWorker(), sc2::Unit::Alliance::Self).size());
    // Variable used for measuring when we want to expand or not.
    int optimal_workers = 0;

    //Counting plans of expansion
    int scheduled_ccs = static_cast<int>(builder_->CountScheduledStructures(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER));

    for (const auto i : m_planner_queue) {
        if (i == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER)
            scheduled_ccs++;
    }

    if (scheduled_ccs == 0) {
        // Calculate Optimal Workers
        for (auto& cc : command_centers)
            optimal_workers += static_cast<int>(std::ceil(1.5f * cc->ideal_harvesters));    // Assume ~50% overproduction for mining
        for (auto& refinery : refineries)
            optimal_workers += refinery->ideal_harvesters;

        //If we greed, we want to increase the rate at which we expand by a multiple of 2.
        optimal_workers = static_cast<int>(std::ceil(optimal_workers / greed_modifier));

        //If we have more workers than the optimal amount we want to expand in order to place them at new commandcenters
        if (num_workers >= optimal_workers) {
            if (playstyle == PlayStyle::greedy) {
                m_planner_queue.emplace_front(sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS);
                m_planner_queue.emplace_front(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
            }
            else {
                m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER);
                m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS);
            }
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
            m_planner_queue.emplace_back(sc2::UNIT_TYPEID::TERRAN_REFINERY);
        }
    }
}

void Governor::PrioritizeCommandCenter() {
    int start_to_sort = 0;
    for (auto it = m_planner_queue.begin(); it != m_planner_queue.end(); ++it) {
        if (!start_to_sort && *it != sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER)
            start_to_sort = 1;

        if (*it == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER && start_to_sort) {
            m_planner_queue.splice(m_planner_queue.begin(), m_planner_queue, it);
            break;
        }
    }
}

int Governor::CountTotalStructures(Builder* builder_, sc2::UNIT_TYPEID type) {
    int total_structures = static_cast<int>(builder_->CountScheduledStructures(type));

    for (const auto i : m_planner_queue) {
        if (i == type)
            total_structures++;
    }

    total_structures += static_cast<int>(gAPI->observer().GetUnits(IsUnit(type, true),
        sc2::Unit::Alliance::Self).size());

    return total_structures;
}

int Governor::CountTotalUnits(Builder* builder_, sc2::UNIT_TYPEID type) {
    int total_units = static_cast<int>(builder_->CountScheduledTrainings(type));

    for (const auto i : m_planner_queue) {
        if (i == type)
            total_units++;
    }

    total_units += static_cast<int>(gAPI->observer().GetUnits(IsUnit(type, true),
        sc2::Unit::Alliance::Self).size());

    return total_units;

}

void Governor::OnUnitIdle(Unit *unit_, Builder *builder_) {
    auto unit_classes = gReasoner->GetNeededUnitClasses();
    bool anti_air = false;
    for (auto i : unit_classes) {
        if (i == UnitClass::anti_air)
            anti_air = true;
    }

    int num_of_hellbats = CountTotalUnits(builder_, sc2::UNIT_TYPEID::TERRAN_HELLIONTANK) +
        CountTotalUnits(builder_, sc2::UNIT_TYPEID::TERRAN_HELLION);
    int num_of_medivacs = CountTotalUnits(builder_, sc2::UNIT_TYPEID::TERRAN_MEDIVAC);

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
            break;
        case sc2::UNIT_TYPEID::TERRAN_FACTORY:
            //TODO sometimes we might want to produce cyclons
            if (HasAddon(sc2::UNIT_TYPEID::TERRAN_TECHLAB)(*unit_)) {
                int num_of_thors = CountTotalUnits(builder_, sc2::UNIT_TYPEID::TERRAN_THOR);
                int num_of_tanks = CountTotalUnits(builder_, sc2::UNIT_TYPEID::TERRAN_SIEGETANK);

                if (num_of_tanks < 1 / ThorsToTanksRatio && !anti_air) {
                    builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_SIEGETANK, false, unit_);
                    gHistory.info() << "Schedule Siegetank training" << std::endl;
                    return;
                }
                //Build anti-air if army ratio is not fullfilled
                if (num_of_thors == 0 || anti_air ||
                   (( num_of_thors + num_of_tanks) / static_cast<float>(num_of_thors)) < ThorsToTanksRatio ) {

                    builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_THOR, false, unit_);
                    gHistory.info() << "Schedule Thor training" << std::endl;
                    return;
                }

                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_SIEGETANK, false, unit_);
                gHistory.info() << "Schedule Siegetank training" << std::endl;
                return;
            }
            else if (HasAddon(sc2::UNIT_TYPEID::TERRAN_REACTOR)(*unit_)) {
                //TODO We don't always want to schedule 2 units here...
                if (anti_air) {
                    builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_WIDOWMINE, false, unit_);
                    builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_WIDOWMINE, false, unit_);
                    gHistory.info() << "Schedule double Widowmine training" << std::endl;
                    return;
                }
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_HELLION, false, unit_);
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_HELLION, false, unit_);
                gHistory.info() << "Schedule double Hellion training" << std::endl;
                return;
            } else {
                builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_HELLION, false, unit_);
                gHistory.info() << "Schedule Hellion training" << std::endl;
                // Naked
            }
            break;
         case sc2::UNIT_TYPEID::TERRAN_STARPORT:
             if (HasAddon(sc2::UNIT_TYPEID::TERRAN_TECHLAB)(*unit_)) {
                 int num_of_ravens = CountTotalUnits(builder_, sc2::UNIT_TYPEID::TERRAN_RAVEN);
                 if (num_of_ravens > OptimalNumOfRavens) { // If we have enough ravens, build other units
                     if (num_of_medivacs == 0 ||
                         (num_of_medivacs / static_cast<float>(num_of_hellbats)) < MedivacsToHellbatsRatio) {
                         builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_MEDIVAC, false, unit_);
                         gHistory.info() << "Schedule Medivac training" << std::endl;
                         return;
                     }
                     builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, false, unit_);
                     gHistory.info() << "Schedule Hellion training" << std::endl;
                     return;
                 }
                     // We want to use ravens for spotting stealth units.
                 builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_RAVEN, false, unit_);
                 gHistory.info() << "Schedule Raven training" << std::endl;
             }
             else if (HasAddon(sc2::UNIT_TYPEID::TERRAN_REACTOR)(*unit_)) {
                 if (num_of_medivacs == 0 ||
                     (num_of_medivacs / static_cast<float>(num_of_hellbats)) < MedivacsToHellbatsRatio) {
                     builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_MEDIVAC, false, unit_);
                     builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_MEDIVAC, false, unit_);
                     gHistory.info() << "Schedule double Medivac training" << std::endl;
                     return;
                 }
                 builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, false, unit_);
                 builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, false, unit_);
                 gHistory.info() << "Schedule double Hellion training" << std::endl;
             }
             else { //case of no addon
                 if (num_of_medivacs == 0 ||
                     (num_of_medivacs / static_cast<float>(num_of_hellbats)) < MedivacsToHellbatsRatio) {
                     builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_MEDIVAC, false, unit_);
                     gHistory.info() << "Schedule Medivac training" << std::endl;
                     return;
                 }
                 builder_->ScheduleTraining(sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, false, unit_);
                 gHistory.info() << "Schedule Hellion training" << std::endl;
             }

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

    int thor_mineral = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_THOR).mineral_cost;
    int thor_vespene = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_THOR).vespene_cost;
    float thor_build_time = gAPI->observer().GetUnitTypeData(sc2::UNIT_TYPEID::TERRAN_THOR).build_time;

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
        tank_mineral / tank_build_time * (1 - ThorsToTanksRatio) +
        CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB) *
        thor_mineral / thor_build_time * ThorsToTanksRatio;
    vespene_consumption += CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB) *
        (tank_vespene / tank_build_time) * (1 - ThorsToTanksRatio) +
        CountTotalStructures(builder_, sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB) *
        thor_vespene / thor_build_time * ThorsToTanksRatio;

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
