// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "API.h"
#include "Converter.h"
#include "Helpers.h"
#include "Historican.h"
#include "plugins/micro/MicroPlugin.h"

#include <sc2api/sc2_map_info.h>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace API {

static std::unordered_map<sc2::ABILITY_ID, sc2::UNIT_TYPEID> AbilityToUnitMap;
static std::unordered_map<sc2::ABILITY_ID, sc2::UPGRADE_ID> AbilityToUpgradeMap;

Action::Action(sc2::ActionInterface* action_): m_action(action_) {
}

void Action::Build(Order& order_, bool queue_) {
    m_action->UnitCommand(order_.assignee, order_.ability_id, queue_);
    order_.assignee->m_order_queued_in_current_step = true;
}

void Action::Build(Order& order_, const Unit* unit_, bool queue_) {
    m_action->UnitCommand(order_.assignee, order_.ability_id, unit_, queue_);
    order_.assignee->m_order_queued_in_current_step = true;
}

void Action::Build(Order& order_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(order_.assignee, order_.ability_id, point_, queue_);
    order_.assignee->m_order_queued_in_current_step = true;
}

void Action::Attack(Unit* unit_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(unit_, sc2::ABILITY_ID::ATTACK_ATTACK, point_, queue_);
    unit_->m_order_queued_in_current_step = true;
}

void Action::Attack(Units& units_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(units_.ToAPI(), sc2::ABILITY_ID::ATTACK_ATTACK, point_, queue_);
    for (auto& unit : units_) {
        unit->m_order_queued_in_current_step = true;
    }
}

void Action::Attack(Unit* unit_, const Unit* target_, bool queue_) {
    m_action->UnitCommand(unit_, sc2::ABILITY_ID::ATTACK_ATTACK, target_, queue_);
    unit_->m_order_queued_in_current_step = true;
}

void Action::Attack(Units& units_, const Unit* target_, bool queue_) {
    m_action->UnitCommand(units_.ToAPI(), sc2::ABILITY_ID::ATTACK_ATTACK, target_, queue_);
    for (auto& unit : units_) {
        unit->m_order_queued_in_current_step = true;
    }
}

void Action::MoveTo(Unit* unit_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(unit_, sc2::ABILITY_ID::MOVE, point_, queue_);
    unit_->m_order_queued_in_current_step = true;
}

void Action::MoveTo(Units& units_, const sc2::Point2D& point_, bool queue_) {
    m_action->UnitCommand(units_.ToAPI(), sc2::ABILITY_ID::MOVE, point_, queue_);
    for (auto& unit : units_) {
        unit->m_order_queued_in_current_step = true;
    }
}

void Action::Stop(Unit* unit_, bool queue_) {
    m_action->UnitCommand(unit_, sc2::ABILITY_ID::STOP, queue_);
    unit_->m_order_queued_in_current_step = true;
}

void Action::Stop(Units& units_, bool queue_) {
    m_action->UnitCommand(units_.ToAPI(), sc2::ABILITY_ID::STOP, queue_);
    for (auto& unit : units_) {
        unit->m_order_queued_in_current_step = true;
    }
}

void Action::Cast(Unit* assignee_, sc2::ABILITY_ID ability_, bool queue_) {
    m_action->UnitCommand(assignee_, ability_, queue_);
    assignee_->m_order_queued_in_current_step = true;
}

void Action::Cast(Unit* assignee_, sc2::ABILITY_ID ability_,
    const Unit* target_, bool queue_) {
    m_action->UnitCommand(assignee_, convert::ToAbilityID(ability_), target_, queue_);
    assignee_->m_order_queued_in_current_step = true;
}

void Action::Cast(Unit* assignee_, sc2::ABILITY_ID ability_,
    const sc2::Point2D& point, bool queue_) {
    m_action->UnitCommand(assignee_, convert::ToAbilityID(ability_), point, queue_);
    assignee_->m_order_queued_in_current_step = true;
}

void Action::LowerDepot(Unit* assignee_) {
    m_action->UnitCommand(assignee_, sc2::ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
    assignee_->m_order_queued_in_current_step = true;
}

void Action::RaiseDepot(Unit* assignee_) {
    m_action->UnitCommand(assignee_, sc2::ABILITY_ID::MORPH_SUPPLYDEPOT_RAISE);
    assignee_->m_order_queued_in_current_step = true;
}

void Action::OpenGate(Unit* assignee_) {
    m_action->UnitCommand(assignee_, sc2::ABILITY_ID::MORPH_WARPGATE);
    assignee_->m_order_queued_in_current_step = true;
}

void Action::SendMessage(const std::string& text_) {
    m_action->SendChat(text_);
}

Control::Control(sc2::ControlInterface* control_): m_control(control_) {
}

void Control::SaveReplay() {
    std::stringstream ss;
    auto timestamp = std::time(nullptr);
    auto localtime = *std::localtime(&timestamp);
    ss << "MulleMech_" << std::put_time(&localtime, "%Y%m%d__%H_%S") << ".SC2Replay";
    auto replay_name = ss.str();
    if (m_control->SaveReplay(replay_name))
        gHistory.info() << "Replay saved to " << replay_name << std::endl;
    else
        gHistory.info() << "Failed saving replay!" << std::endl;
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

Unit* Observer::GetUnit(sc2::Tag tag_) const {
    auto unit = m_observer->GetUnit(tag_);
    if (!unit)
        return nullptr;
    return gAPI->WrapUnit(unit);
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
    auto units = m_observer->GetUnits(filter_);
    return Units(units);
}

Units Observer::GetUnits(const sc2::Filter& filter_,
    sc2::Unit::Alliance alliance_) const {
    return Units(m_observer->GetUnits(alliance_, filter_));
}

size_t Observer::CountUnitType(sc2::UNIT_TYPEID type_, bool with_not_finished, bool count_tech_alias) const {
    // TODO: Add some nice solutions for buildings that are the same but differ in ID depending on state
    //       such as flying, burrowed, morphed, etc
    // As the API thinks of depots and lowered depots as different buildings, we handle this as a special case
    // (by actually counting how many supply depots you have when the type_ is TERRAN_SUPPLYDEPOT)
    if (type_ == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
        return m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, with_not_finished)).size() +
                m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED, with_not_finished)).size();
    }
    // Same with Orbital Command, Planetary Fortress and Command Center
    if (count_tech_alias && type_ == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER) {
        return m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER, with_not_finished)).size() +
            m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND, with_not_finished)).size() +
            m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS, with_not_finished)).size();
    }

    return m_observer->GetUnits(sc2::Unit::Alliance::Self, IsUnit(type_, with_not_finished)).size();
}

const std::vector<sc2::UpgradeID>& Observer::GetUpgrades() const {
    return m_observer->GetUpgrades();
}

bool Observer::HasUpgrade(sc2::UPGRADE_ID upgrade_id_) const {
    for (auto& upgrade : GetUpgrades()) {
        if (upgrade == upgrade_id_) {
            return true;
        }
    }
    return false;
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

float Observer::GetMineralIncomeRate() const {
    return m_observer->GetScore().score_details.collection_rate_minerals;
}

float Observer::GetVespeneIncomeRate() const {
    return m_observer->GetScore().score_details.collection_rate_vespene;
}

sc2::UnitTypeData* Observer::GetUnitTypeData(sc2::UNIT_TYPEID id_) const {
    static std::unordered_map<sc2::UNIT_TYPEID, std::unique_ptr<sc2::UnitTypeData>> data_cache;

    auto itr = data_cache.find(id_);
    if (itr != data_cache.end())
        return itr->second.get();

    data_cache.emplace(id_, std::make_unique<sc2::UnitTypeData>());
    sc2::UnitTypeData* data = data_cache.find(id_)->second.get();

    *data = m_observer->GetUnitTypeData()[convert::ToUnitTypeID(id_)];

    switch (id_) {
        // NOTE (alkurbatov): Unfortunally SC2 API returns wrong mineral cost
        // and tech_requirement for orbital command, planetary fortress,
        // lair, hive and greater spire.
        // so we use a workaround.
        // See https://github.com/Blizzard/s2client-api/issues/191
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
            data->mineral_cost = 150;
            data->tech_requirement = sc2::UNIT_TYPEID::TERRAN_BARRACKS;
            break;

        case sc2::UNIT_TYPEID::ZERG_GREATERSPIRE:
            data->mineral_cost = 100;
            data->vespene_cost = 150;
            data->tech_requirement = sc2::UNIT_TYPEID::ZERG_HIVE;
            break;

        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
            data->mineral_cost = 150;
            data->tech_requirement = sc2::UNIT_TYPEID::TERRAN_ENGINEERINGBAY;
            break;

        case sc2::UNIT_TYPEID::ZERG_BANELING:
            data->mineral_cost = 25;
            data->food_required = 0.0f;
            data->tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_ZERGLING);
            break;

        case sc2::UNIT_TYPEID::ZERG_BROODLORD:
            data->mineral_cost = 150;
            data->vespene_cost = 150;
            data->food_required = 2.0f;
            data->tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_CORRUPTOR);
            data->tech_requirement = sc2::UNIT_TYPEID::ZERG_GREATERSPIRE;
            break;

        case sc2::UNIT_TYPEID::ZERG_LAIR:
            data->mineral_cost = 150;
            data->tech_requirement = sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL;
            break;

        case sc2::UNIT_TYPEID::ZERG_OVERSEER:
            data->mineral_cost = 50;
            data->tech_requirement = sc2::UNIT_TYPEID::ZERG_LAIR;
            break;

        case sc2::UNIT_TYPEID::ZERG_RAVAGER:
            data->mineral_cost = 25;
            data->vespene_cost = 75;
            data->food_required = 1.0f;
            data->tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_ROACH);
            data->tech_requirement = sc2::UNIT_TYPEID::ZERG_ROACHWARREN;
            break;

        case sc2::UNIT_TYPEID::ZERG_HIVE:
            data->mineral_cost = 200;
            data->vespene_cost = 150;
            data->tech_requirement = sc2::UNIT_TYPEID::ZERG_INFESTATIONPIT;
            break;

        case sc2::UNIT_TYPEID::ZERG_LURKERMP:
            data->mineral_cost = 50;
            data->vespene_cost = 100;
            data->ability_id = sc2::ABILITY_ID::MORPH_LURKER;
            data->food_required = 1.0f;
            data->tech_alias.emplace_back(sc2::UNIT_TYPEID::ZERG_HYDRALISK);
            data->tech_requirement = sc2::UNIT_TYPEID::ZERG_LURKERDENMP;
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
            data->mineral_cost -= 50;
            break;

        // NOTE (alkurbatov): There is no sense in summoning protoss buildings
        // without a pylon.
        case sc2::UNIT_TYPEID::PROTOSS_FORGE:
        case sc2::UNIT_TYPEID::PROTOSS_GATEWAY:
            data->tech_requirement = sc2::UNIT_TYPEID::PROTOSS_PYLON;
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

sc2::UNIT_TYPEID Observer::GetUnitConstructedFromAbility(sc2::ABILITY_ID id_) const {
    auto itr = AbilityToUnitMap.find(id_);
    if (itr == AbilityToUnitMap.end())
        return sc2::UNIT_TYPEID::INVALID;
    return itr->second;
}

sc2::UPGRADE_ID Observer::GetUpgradeFromAbility(sc2::ABILITY_ID id_) const {
    auto itr = AbilityToUpgradeMap.find(id_);
    if (itr == AbilityToUpgradeMap.end())
        return sc2::UPGRADE_ID::INVALID;
    return itr->second;
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

sc2::Visibility Observer::GetVisibility(const sc2::Point2D& pos_) const {
    return m_observer->GetVisibility(pos_);
}

const std::vector<sc2::PlayerResult>& Observer::GetResults() const {
    return m_observer->GetResults();
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

float Query::PathingDistance(const Unit* start_, const sc2::Point2D& end_) const {
    return m_query->PathingDistance(start_, end_);
}

std::vector<float> Query::PathingDistances(const std::vector<sc2::QueryInterface::PathingQuery>& queries_) const {
    return m_query->PathingDistance(queries_);
}

sc2::AvailableAbilities Query::GetAbilitiesForUnit(const Unit* unit_, bool ignore_resource_requirements_) const {
    return m_query->GetAbilitiesForUnit(unit_, ignore_resource_requirements_);
}

Interface::Interface(sc2::ActionInterface* action_,
    sc2::ControlInterface* control_, sc2::DebugInterface* debug_,
    const sc2::ObservationInterface* observer_, sc2::QueryInterface* query_):
    m_action(action_), m_control(control_), m_debug(debug_),
    m_observer(observer_), m_query(query_) {
}

void Interface::Init() {
    // Make a mapping of ability -> unit, for abilities that construct units
    const auto& unit_datas = m_observer->GetUnitTypeData();
    for (auto& data : unit_datas) {
        if (data.ability_id != sc2::ABILITY_ID::INVALID)
            AbilityToUnitMap[data.ability_id] = data.unit_type_id;
    }
    // Make a mapping of ability -> upgrade
    const auto& upgrade_datas = m_observer->GetUpgradeData();
    for (auto& data : upgrade_datas) {
        if (data.ability_id != sc2::ABILITY_ID::INVALID)
            AbilityToUpgradeMap[data.ability_id] = sc2::UpgradeID(data.upgrade_id);
    }
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

Unit* Interface::WrapUnit(const sc2::Unit* unit_) {
    assert(unit_ != nullptr);

    auto itr = m_unitObjects.find(unit_->tag);
    if (itr == m_unitObjects.end()) {
        m_unitObjects[unit_->tag] = Unit::Make(*unit_);
        return m_unitObjects[unit_->tag].get();
    }

    itr->second->UpdateAPIData(*unit_);
    return itr->second.get();
}

void Interface::OnStep() {
    for (auto& pair : m_unitObjects)
        pair.second->IsInVision = false; // If unit went into FoW it'll no longer be in GetUnits()

    sc2::Units units = observer().m_observer->GetUnits();
    for (const sc2::Unit* unit : units) {
        auto itr = m_unitObjects.find(unit->tag);
        if (itr == m_unitObjects.end()) {
            m_unitObjects[unit->tag] = Unit::Make(*unit);
        } else {
            itr->second->IsInVision = true;
            itr->second->m_order_queued_in_current_step = false;
            itr->second->UpdateAPIData(*unit);
        }
    }
}

}  // namespace API

std::unique_ptr<API::Interface> gAPI;
