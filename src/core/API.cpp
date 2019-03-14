// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "API.h"
#include "Converter.h"
#include "Helpers.h"
#include "objects/GameObject.h"

#include <sc2api/sc2_map_info.h>

namespace API {

Action::Action(sc2::ActionInterface* action_): m_action(action_) {
}

void Action::Build(const Order& order_, bool queue_) {
    sc2::Unit unit = GameObject::ToUnit(order_.assignee);
    m_action->UnitCommand(&unit, order_.ability_id, queue_);
}

void Action::Build(const Order& order_, const sc2::Unit* unit_, bool queue_) {
    sc2::Unit unit = GameObject::ToUnit(order_.assignee);
    m_action->UnitCommand(&unit, order_.ability_id, unit_, queue_);
}

void Action::Build(const Order& order_, const sc2::Point2D& point_, bool queue_) {
    sc2::Unit unit = GameObject::ToUnit(order_.assignee);
    m_action->UnitCommand(&unit, order_.ability_id, point_, queue_);
}

void Action::Attack(const sc2::Unit& unit_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(&unit_, sc2::ABILITY_ID::ATTACK_ATTACK, point_, queue_);
}

void Action::Attack(const sc2::Units& units_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(units_, sc2::ABILITY_ID::ATTACK_ATTACK, point_, queue_);
}

void Action::MoveTo(const sc2::Unit& unit_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(&unit_, sc2::ABILITY_ID::MOVE, point_, queue_);
}

void Action::MoveTo(const sc2::Units& units_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(units_, sc2::ABILITY_ID::MOVE, point_, queue_);
}

void Action::Stop(const sc2::Unit& unit_, bool queue_) {
    m_action->UnitCommand(&unit_, sc2::ABILITY_ID::STOP, queue_);
}

void Action::Stop(const sc2::Units& units_, bool queue_) {
    m_action->UnitCommand(units_, sc2::ABILITY_ID::STOP, queue_);
}

void Action::Cast(const sc2::Unit& assignee_, sc2::ABILITY_ID ability_,
    const sc2::Unit& target_, bool queue_) {
    m_action->UnitCommand(&assignee_, convert::ToAbilityID(ability_), &target_, queue_);
}

void Action::LowerDepot(const sc2::Unit& assignee_) {
    m_action->UnitCommand(&assignee_, sc2::ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
}

void Action::RaiseDepot(const sc2::Unit& assignee_) {
    m_action->UnitCommand(&assignee_, sc2::ABILITY_ID::MORPH_SUPPLYDEPOT_RAISE);
}

void Action::OpenGate(const sc2::Unit& assignee_) {
    m_action->UnitCommand(&assignee_, sc2::ABILITY_ID::MORPH_WARPGATE);
}

void Action::SendMessage(const std::string& text_) {
    m_action->SendChat(text_);
}

Control::Control(sc2::ControlInterface* control_): m_control(control_) {
}

void Control::SaveReplay() {
    m_control->SaveReplay("LastReplay.SC2Replay");
}

Debug::Debug(sc2::DebugInterface* debug_): m_debug(debug_) {
}

void Debug::DrawText(const std::string& message_) const {
    m_debug->DebugTextOut(message_);
}

void Debug::DrawText(const std::string& message_, const sc2::Point3D& pos_) const {
    m_debug->DebugTextOut(message_, pos_);
}

void Debug::DrawSphere(const sc2::Point3D& center_, float radius_) const {
    m_debug->DebugSphereOut(center_, radius_);
}

void Debug::DrawBox(const sc2::Point3D& min_, const sc2::Point3D& max_) const {
    m_debug->DebugBoxOut(min_, max_);
}

void Debug::DrawLine(const sc2::Point3D& start_, const sc2::Point3D& end_) const {
    m_debug->DebugLineOut(start_, end_);
}

void Debug::EndGame() const {
    m_debug->DebugEndGame(true);
    SendDebug();
}

void Debug::SendDebug() const {
    m_debug->SendDebug();
}

Observer::Observer(const sc2::ObservationInterface* observer_):
    m_observer(observer_) {
}

const sc2::Unit* Observer::GetUnit(sc2::Tag tag_) const {
    return m_observer->GetUnit(tag_);
}

Units Observer::GetUnits() const {
    return Units(m_observer->GetUnits());
}

Units Observer::GetUnits(sc2::Unit::Alliance alliance_) const {
    return Units(m_observer->GetUnits(alliance_));
}

Units Observer::GetUnits(const sc2::Filter& filter_) const {
    // NOTE: The documentation for this function is wrong, in sc2_client.cc it
    //       does ForEachExistingUnit, and only applies the filter, it doesn't
    //       force alliance Self
    return Units(m_observer->GetUnits(filter_));
}

Units Observer::GetUnits(const sc2::Filter& filter_,
    sc2::Unit::Alliance alliance_) const {
    return Units(m_observer->GetUnits(alliance_, filter_));
}

size_t Observer::CountUnitType(sc2::UNIT_TYPEID type_, bool with_not_finished) const {
    // As the API thinks of depots and lowered depots as different buildings, we handle this as a special case
    // (by actually counting how many supply depots you have when the type_ is TERRAN_SUPPLYDEPOT)
    if (type_ == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
        return m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, with_not_finished)).size() +
                m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED, with_not_finished)).size();
    }
    return m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(type_, with_not_finished)).size();
}

const sc2::GameInfo& Observer::GameInfo() const {
    return m_observer->GetGameInfo();
}

sc2::Point3D Observer::StartingLocation() const {
    return m_observer->GetStartLocation();
}

int32_t Observer::GetFoodCap() const {
    return m_observer->GetFoodCap();
}

int32_t Observer::GetFoodUsed() const {
    return m_observer->GetFoodUsed();
}

int32_t Observer::GetMinerals() const {
    return m_observer->GetMinerals();
}

int32_t Observer::GetVespene() const {
    return m_observer->GetVespene();
}

float Observer::GetAvailableFood() const {
    return static_cast<float>(GetFoodCap() - GetFoodUsed());
}

sc2::UnitTypeData Observer::GetUnitTypeData(sc2::UNIT_TYPEID id_) const {
    sc2::UnitTypeData data = m_observer->GetUnitTypeData()[convert::ToUnitTypeID(id_)];

    switch (id_) {
        // NOTE (alkurbatov): Unfortunally SC2 API returns wrong mineral cost
        // and tech_requirement for orbital command, planetary fortress,
        // lair, hive and greater spire.
        // so we use a workaround.
        // See https://github.com/Blizzard/s2client-api/issues/191
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
            data.mineral_cost = 150;
            data.tech_requirement = sc2::UNIT_TYPEID::TERRAN_BARRACKS;
            break;

        case sc2::UNIT_TYPEID::ZERG_GREATERSPIRE:
            data.mineral_cost = 100;
            data.vespene_cost = 150;
            data.tech_requirement = sc2::UNIT_TYPEID::ZERG_HIVE;
            break;

        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
            data.mineral_cost = 150;
            data.tech_requirement = sc2::UNIT_TYPEID::TERRAN_ENGINEERINGBAY;
            break;

        case sc2::UNIT_TYPEID::ZERG_BANELING:
            data.mineral_cost = 25;
            data.food_required = 0.0f;
            data.tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_ZERGLING);
            break;

        case sc2::UNIT_TYPEID::ZERG_BROODLORD:
            data.mineral_cost = 150;
            data.vespene_cost = 150;
            data.food_required = 2.0f;
            data.tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_CORRUPTOR);
            data.tech_requirement = sc2::UNIT_TYPEID::ZERG_GREATERSPIRE;
            break;

        case sc2::UNIT_TYPEID::ZERG_LAIR:
            data.mineral_cost = 150;
            data.tech_requirement = sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL;
            break;

        case sc2::UNIT_TYPEID::ZERG_OVERSEER:
            data.mineral_cost = 50;
            data.tech_requirement = sc2::UNIT_TYPEID::ZERG_LAIR;
            break;

        case sc2::UNIT_TYPEID::ZERG_RAVAGER:
            data.mineral_cost = 25;
            data.vespene_cost = 75;
            data.food_required = 1.0f;
            data.tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_ROACH);
            data.tech_requirement = sc2::UNIT_TYPEID::ZERG_ROACHWARREN;
            break;

        case sc2::UNIT_TYPEID::ZERG_HIVE:
            data.mineral_cost = 200;
            data.vespene_cost = 150;
            data.tech_requirement = sc2::UNIT_TYPEID::ZERG_INFESTATIONPIT;
            break;

        case sc2::UNIT_TYPEID::ZERG_LURKERMP:
            data.mineral_cost = 50;
            data.vespene_cost = 100;
            data.ability_id = sc2::ABILITY_ID::MORPH_LURKER;
            data.food_required = 1.0f;
            data.tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_HYDRALISK);
            data.tech_requirement = sc2::UNIT_TYPEID::ZERG_LURKERDENMP;
            break;

        // NOTE (alkurbatov): By some reason all zerg buildings
        // include drone mineral cost.
        case sc2::UNIT_TYPEID::ZERG_BANELINGNEST:
        case sc2::UNIT_TYPEID::ZERG_EVOLUTIONCHAMBER:
        case sc2::UNIT_TYPEID::ZERG_EXTRACTOR:
        case sc2::UNIT_TYPEID::ZERG_HYDRALISKDEN:
        case sc2::UNIT_TYPEID::ZERG_INFESTATIONPIT:
        case sc2::UNIT_TYPEID::ZERG_LURKERDENMP:
        case sc2::UNIT_TYPEID::ZERG_ROACHWARREN:
        case sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL:
        case sc2::UNIT_TYPEID::ZERG_SPINECRAWLER:
        case sc2::UNIT_TYPEID::ZERG_SPIRE:
        case sc2::UNIT_TYPEID::ZERG_SPORECRAWLER:
        case sc2::UNIT_TYPEID::ZERG_ULTRALISKCAVERN:
            data.mineral_cost -= 50;
            break;

        // NOTE (alkurbatov): There is no sense in summoning protoss buildings
        // without a pylon.
        case sc2::UNIT_TYPEID::PROTOSS_FORGE:
        case sc2::UNIT_TYPEID::PROTOSS_GATEWAY:
            data.tech_requirement = sc2::UNIT_TYPEID::PROTOSS_PYLON;
            break;

        default:
            break;
    }

    return data;
}

sc2::UpgradeData Observer::GetUpgradeData(sc2::UPGRADE_ID id_) const {
    return m_observer->GetUpgradeData()[convert::ToUpgradeID(id_)];
}

sc2::AbilityData Observer::GetAbilityData(sc2::ABILITY_ID id_) const {
    return m_observer->GetAbilityData()[convert::ToAbilityID(id_)];
}

sc2::Race Observer::GetCurrentRace() const {
    uint32_t id = m_observer->GetPlayerID();
    return m_observer->GetGameInfo().player_info[id - 1].race_actual;
}

const std::vector<sc2::ChatMessage>& Observer::GetChatMessages() const {
    return m_observer->GetChatMessages();
}

uint32_t Observer::GetGameLoop() const {
    return m_observer->GetGameLoop();
}

float Observer::TerrainHeight(const sc2::Point2D& pos_) const
{
    auto& info = m_observer->GetGameInfo();
    sc2::Point2DI posi(static_cast<int>(pos_.x), static_cast<int>(pos_.y));
    if (posi.x < 0 || posi.x >= info.width || posi.y < 0 || posi.y >= info.width)
        return 0.0f;

    assert(static_cast<int>(info.terrain_height.data.size()) == info.width * info.height);
    int encodedHeight = info.terrain_height.data[static_cast<unsigned>(posi.x + ((info.height - 1) - posi.y) * info.width)];
    float decodedHeight = -100.0f + 200.0f * float(encodedHeight) / 255.0f;
    return decodedHeight;
}

Query::Query(sc2::QueryInterface* query_): m_query(query_) {
}

bool Query::CanBePlaced(const Order& order_, const sc2::Point2D& point_) {
    return m_query->Placement(order_.ability_id, point_);
}

std::vector<bool> Query::CanBePlaced(
    const std::vector<sc2::QueryInterface::PlacementQuery>& queries_) {
    return m_query->Placement(queries_);
}

float Query::PathingDistance(const sc2::Point2D& start_, const sc2::Point2D& end_) const {
    return m_query->PathingDistance(start_, end_);
}

float Query::PathingDistance(const sc2::Unit& start_, const sc2::Point2D& end_) const {
    return m_query->PathingDistance(&start_, end_);
}

std::vector<float> Query::PathingDistances(const std::vector<sc2::QueryInterface::PathingQuery>& queries_) const {
    return m_query->PathingDistance(queries_);
}

Interface::Interface(sc2::ActionInterface* action_,
    sc2::ControlInterface* control_, sc2::DebugInterface* debug_,
    const sc2::ObservationInterface* observer_, sc2::QueryInterface* query_):
    m_action(action_), m_control(control_), m_debug(debug_),
    m_observer(observer_), m_query(query_) {
}

Action Interface::action() const {
    return Action(m_action);
}

Control Interface::control() const {
    return Control(m_control);
}

Debug Interface::debug() const {
    return Debug(m_debug);
}

Observer Interface::observer() const {
    return Observer(m_observer);
}

Query Interface::query() const {
    return Query(m_query);
}

}  // namespace API

std::unique_ptr<API::Interface> gAPI;
