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

#include <memory>

namespace API {

struct Action {
    explicit Action(sc2::ActionInterface* action_);

    void Build(const Order& order_, bool queue_ = false);
    void Build(const Order& order_, const sc2::Unit* unit_, bool queue_ = false);
    void Build(const Order& order_, const sc2::Point2D& point_, bool queue_ = false);

    void Attack(const sc2::Unit& unit_, const sc2::Point2D& point_, bool queue_ = false);
    void Attack(const sc2::Units& units_, const sc2::Point2D& point_, bool queue_ = false);

    void MoveTo(const sc2::Unit& unit_, const sc2::Point2D& point_, bool queue_ = false);
    void MoveTo(const sc2::Units& units_, const sc2::Point2D& point_, bool queue_ = false);

    void Stop(const sc2::Unit& unit_, bool queue_ = false);
    void Stop(const sc2::Units& units_, bool queue_ = false);

    void Cast(const sc2::Unit& assignee_, sc2::ABILITY_ID ability_,
        const sc2::Unit& target_, bool queue_ = false);

    void LowerDepot(const sc2::Unit& assignee_);
    void RaiseDepot(const sc2::Unit& assignee_);
    void OpenGate(const sc2::Unit& assignee_);

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
    explicit Observer(const sc2::ObservationInterface* observer_);

    const sc2::Unit* GetUnit(sc2::Tag tag_) const;

    // Get all visible units
    Units GetUnits() const;

    // Get Units by alliance
    Units GetUnits(sc2::Unit::Alliance alliance_) const;

    // Get Units by filter
    Units GetUnits(const sc2::Filter& filter_) const;

    // Get Units by alliance and a filter
    Units GetUnits(const sc2::Filter& filter_, sc2::Unit::Alliance alliance_) const;

    size_t CountUnitType(sc2::UNIT_TYPEID type_,
        bool with_not_finished = false) const;

    const sc2::GameInfo& GameInfo() const;

    sc2::Point3D StartingLocation() const;

    int32_t GetFoodCap() const;

    int32_t GetFoodUsed() const;

    int32_t GetMinerals() const;

    int32_t GetVespene() const;

    float GetAvailableFood() const;

    sc2::UnitTypeData GetUnitTypeData(sc2::UNIT_TYPEID id_) const;

    sc2::UpgradeData GetUpgradeData(sc2::UPGRADE_ID id_) const;

    sc2::AbilityData GetAbilityData(sc2::ABILITY_ID id_) const;

    sc2::Race GetCurrentRace() const;

    const std::vector<sc2::ChatMessage>& GetChatMessages() const;

    uint32_t GetGameLoop() const;

    float TerrainHeight(const sc2::Point2D& pos_) const;

 private:
    const sc2::ObservationInterface* m_observer;
};

struct Query {
    explicit Query(sc2::QueryInterface* query_);

    bool CanBePlaced(const Order& order_, const sc2::Point2D& point_);
    std::vector<bool> CanBePlaced(
        const std::vector<sc2::QueryInterface::PlacementQuery>& queries_);

    float PathingDistance(const sc2::Point2D& start_, const sc2::Point2D& end_) const;
    float PathingDistance(const sc2::Unit& start_, const sc2::Point2D& end_) const;
    std::vector<float> PathingDistances(const std::vector<sc2::QueryInterface::PathingQuery>& queries_) const;

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

    Action action() const;

    Control control() const;

    Debug debug() const;

    Observer observer() const;

    Query query() const;

 private:
    sc2::ActionInterface* m_action;
    sc2::ControlInterface* m_control;
    sc2::DebugInterface* m_debug;
    const sc2::ObservationInterface* m_observer;
    sc2::QueryInterface* m_query;
};

}  // namespace API

extern std::unique_ptr<API::Interface> gAPI;
