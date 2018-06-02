// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Converter.h"
#include "Helpers.h"
#include "World.h"

IsUnit::IsUnit(sc2::UNIT_TYPEID type_, bool with_not_finished):
    m_type(type_), m_build_progress(1.0f) {
    if (with_not_finished)
        m_build_progress = 0.0f;
}

bool IsUnit::operator()(const sc2::Unit& unit_) {
    return unit_.unit_type == m_type &&
        unit_.build_progress >= m_build_progress;
}

bool IsMineralPatch::operator()(const sc2::Unit& unit_) {
    return unit_.mineral_contents > 0;
}

bool IsGeyser::operator()(const sc2::Unit& unit_) {
    return unit_.vespene_contents > 0;
}

bool IsFreeGeyser::operator()(const sc2::Unit& unit_) {
    return IsGeyser()(unit_) && !gWorld->IsOccupied(unit_);
}

bool IsRefinery::operator()(const sc2::Unit& unit_) {
    if (unit_.build_progress != 1.0f)
        return false;

    return unit_.unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
        unit_.unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR ||
        unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY;
}

bool IsWorker::operator()(const sc2::Unit& unit_) {
    return unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_SCV ||
        unit_.unit_type == sc2::UNIT_TYPEID::ZERG_DRONE ||
        unit_.unit_type == sc2::UNIT_TYPEID::PROTOSS_PROBE;
}

bool IsFreeWorker::operator()(const sc2::Unit& unit_) {
    if (!IsWorker()(unit_))
        return false;

    if (unit_.orders.empty())
        return true;

    if (unit_.orders.front().ability_id != sc2::ABILITY_ID::HARVEST_GATHER &&
        unit_.orders.front().ability_id != sc2::ABILITY_ID::HARVEST_RETURN)
        return false;

    return !IsGasWorker()(unit_);
}

bool IsFreeLarva::operator()(const sc2::Unit& unit_) {
    if (unit_.unit_type != sc2::UNIT_TYPEID::ZERG_LARVA)
        return false;

    return unit_.orders.empty();
}

bool IsGasWorker::operator()(const sc2::Unit& unit_) {
    if (!IsWorker()(unit_))
        return false;

    if (unit_.orders.empty())
        return false;

    if (unit_.orders.front().ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
        if (unit_.buffs.empty())
            return false;

        return unit_.buffs.front() == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGAS;
    }

    if (unit_.orders.front().ability_id == sc2::ABILITY_ID::HARVEST_GATHER)
        return gWorld->IsTargetOccupied(unit_.orders.front());

    return false;
}

bool IsBuildingOrder::operator()(const sc2::UnitOrder& order_) {
    switch (convert::ToAbilityID(order_.ability_id)) {
        case sc2::ABILITY_ID::BUILD_COMMANDCENTER:
        case sc2::ABILITY_ID::BUILD_SUPPLYDEPOT:
        case sc2::ABILITY_ID::BUILD_REFINERY:
        case sc2::ABILITY_ID::BUILD_BARRACKS:
        case sc2::ABILITY_ID::BUILD_ENGINEERINGBAY:
        case sc2::ABILITY_ID::BUILD_MISSILETURRET:
        case sc2::ABILITY_ID::BUILD_BUNKER:
        case sc2::ABILITY_ID::BUILD_SENSORTOWER:
        case sc2::ABILITY_ID::BUILD_GHOSTACADEMY:
        case sc2::ABILITY_ID::BUILD_FACTORY:
        case sc2::ABILITY_ID::BUILD_STARPORT:
        case sc2::ABILITY_ID::BUILD_ARMORY:
        case sc2::ABILITY_ID::BUILD_FUSIONCORE:
        case sc2::ABILITY_ID::BUILD_SPAWNINGPOOL:
        case sc2::ABILITY_ID::BUILD_EXTRACTOR:
            return true;

        default:
            return false;
    }
}

bool IsCommandCenter::operator()(const sc2::Unit& unit_) {
    return unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS;
}

bool IsFreeCommandCenter::operator()(const sc2::Unit& unit_) {
    return unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER &&
        unit_.orders.empty();
}

IsOrdered::IsOrdered(sc2::UNIT_TYPEID type_): m_type(type_) {
}

bool IsOrdered::operator()(const Order& order_) {
    return order_.data.unit_type_id == m_type;
}
