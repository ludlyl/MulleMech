// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Order.h"
#include "Units.h"

#include <sc2api/sc2_gametypes.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_control_interfaces.h>
#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_score.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace API {

constexpr float StepsPerSecond = 22.4f;
constexpr float OrbitalScanCost = 50.0f;
constexpr float OrbitalMuleCost = 50.0f;
constexpr float OrbitalScanRadius = 12.3f;

typedef std::function<bool(const Unit* unit)> Filter;

struct Action {
    explicit Action(sc2::ActionInterface* action_);

    void Build(Order& order_, bool queue_ = false);
    void Build(Order& order_, const Unit* unit_, bool queue_ = false);
    void Build(Order& order_, const sc2::Point2D& point_, bool queue_ = false);

    void Attack(Unit* unit_, const sc2::Point2D& point_, bool queue_ = false);
    void Attack(Units& units_, const sc2::Point2D& point_, bool queue_ = false);
    void Attack(Unit* unit_, const Unit* target_, bool queue_ = false);
    void Attack(Units& units_, const Unit* target_, bool queue_ = false);

    void MoveTo(Unit* unit_, const sc2::Point2D& point_, bool queue_ = false);
    void MoveTo(Units& units_, const sc2::Point2D& point_, bool queue_ = false);

    void Stop(Unit* unit_, bool queue_ = false);
    void Stop(Units& units_, bool queue_ = false);

    void Cast(Unit* assignee_, sc2::ABILITY_ID ability_, bool queue_ = false);
    void Cast(Unit* assignee_, sc2::ABILITY_ID ability_,
        const Unit* target_, bool queue_ = false);
    void Cast(Unit* assignee_, sc2::ABILITY_ID ability_,
              const sc2::Point2D& point, bool queue_ = false);

    void LowerDepot(Unit* assignee_);
    void RaiseDepot(Unit* assignee_);
    void OpenGate(Unit* assignee_);

    void SendMessage(const std::string& text_);

private:
    sc2::ActionInterface* m_action;
};

struct Control {
    explicit Control(sc2::ControlInterface* control_);

    void SaveReplay();

private:
    sc2::ControlInterface* m_control;
};

struct Debug {
    explicit Debug(sc2::DebugInterface* debug_);

    void DrawText(const std::string& message_) const;

    void DrawText(const std::string& message_, const sc2::Point3D& pos_) const;

    void DrawSphere(const sc2::Point3D& center_, float radius_) const;

    void DrawBox(const sc2::Point3D& min_, const sc2::Point3D& max_) const;

    void DrawLine(const sc2::Point3D& start_, const sc2::Point3D& end_) const;

    void EndGame() const;

    void SendDebug() const;

private:
    sc2::DebugInterface* m_debug;
};

struct Observer {
    explicit Observer(const sc2::ObservationInterface* observer_,
                      std::unordered_map<sc2::Tag, std::unique_ptr<Unit>>& unit_map_,
                      std::vector<Unit*>& last_step_units_);

    Unit* GetUnit(sc2::Tag tag_) const;

    // Get all visible units
    Units GetUnits() const;

    // Get Units by alliance
    Units GetUnits(sc2::Unit::Alliance alliance_) const;

    // Get Units by filter
    Units GetUnits(const Filter& filter_) const;

    // Get Units by alliance and a filter
    Units GetUnits(const Filter& filter_, sc2::Unit::Alliance alliance_) const;

    // Count how many we have of said unit type, NOTE: This only count or own units!
    size_t CountUnitType(sc2::UNIT_TYPEID type_, bool with_not_finished = false, bool count_tech_alias = true) const;

    const std::vector<sc2::UpgradeID>& GetUpgrades() const;

    bool HasUpgrade(sc2::UPGRADE_ID upgrade_id_) const;

    const sc2::GameInfo& GameInfo() const;

    sc2::Point3D StartingLocation() const;

    int32_t GetFoodCap() const;

    int32_t GetFoodUsed() const;

    int32_t GetMinerals() const;

    int32_t GetVespene() const;

    float GetAvailableFood() const;

    //returns Minerals/min
    float GetMineralIncomeRate() const;

    //returns Vespene/min
    float GetVespeneIncomeRate() const;

    sc2::UnitTypeData* GetUnitTypeData(sc2::UNIT_TYPEID id_);

    sc2::UpgradeData GetUpgradeData(sc2::UPGRADE_ID id_) const;

    sc2::AbilityData GetAbilityData(sc2::ABILITY_ID id_) const;

    sc2::UNIT_TYPEID GetUnitConstructedFromAbility(sc2::ABILITY_ID id_) const;

    sc2::UPGRADE_ID GetUpgradeFromAbility(sc2::ABILITY_ID id_) const;

    sc2::Race GetCurrentRace() const;

    const std::vector<sc2::ChatMessage>& GetChatMessages() const;

    uint32_t GetGameLoop() const;

    float TerrainHeight(const sc2::Point2D& pos_) const;

    sc2::Visibility GetVisibility(const sc2::Point2D& pos_) const;

    const std::vector<sc2::PlayerResult>& GetResults() const;

private:
    void OnUpgradeCompleted();

    friend struct Interface;
    const sc2::ObservationInterface* m_observer;

    std::unordered_map<sc2::UNIT_TYPEID, std::unique_ptr<sc2::UnitTypeData>> m_unit_data_cache;
    std::unordered_map<sc2::Tag, std::unique_ptr<Unit>>& m_unit_map;
    std::vector<Unit*>& m_last_step_units;
};

struct Query {
    explicit Query(sc2::QueryInterface* query_);

    bool CanBePlaced(sc2::ABILITY_ID ability_id_, const sc2::Point2D& point_);
    bool CanBePlaced(const Order& order_, const sc2::Point2D& point_);
    std::vector<bool> CanBePlaced(
        const std::vector<sc2::QueryInterface::PlacementQuery>& queries_);

    float PathingDistance(const sc2::Point2D& start_, const sc2::Point2D& end_) const;
    float PathingDistance(const Unit* start_, const sc2::Point2D& end_) const;
    std::vector<float> PathingDistances(const std::vector<sc2::QueryInterface::PathingQuery>& queries_) const;

    sc2::AvailableAbilities GetAbilitiesForUnit(const Unit* unit_, bool ignore_resource_requirements_ = false) const;

 private:
    sc2::QueryInterface* m_query;
};

struct Interface {
    Interface(
        sc2::ActionInterface* action_,
        sc2::ControlInterface* control_,
        sc2::DebugInterface* debug_,
        const sc2::ObservationInterface* observer_,
        sc2::QueryInterface* query_);

    Action& action() { return m_action; }

    Control& control() { return m_control; }

    Debug& debug() { return m_debug; }

    Observer& observer() { return m_observer; }

    Query& query() { return m_query; }

    // Returned Unit object has life time until end of game and can be saved & accessed without concern
    Unit* WrapAndUpdateUnit(const sc2::Unit* unit_);

    void Init();

    void OnStep();

    void OnUpgradeCompleted();

private:
    Action m_action;
    Control m_control;
    Debug m_debug;
    Observer m_observer;
    Query m_query;
    std::unordered_map<sc2::Tag, std::unique_ptr<Unit>> m_unit_map; // Holds all units that has ever been seen (+ snapshots)
    std::vector<Unit*> m_last_step_units; // Represents the units gotten from sc2::api->GetUnits in the last step
};

}  // namespace API

extern std::unique_ptr<API::Interface> gAPI;
