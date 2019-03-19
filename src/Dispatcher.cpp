// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Dispatcher.h"
#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Brain.h"
#include "core/Helpers.h"
#include "core/Map.h"
#include "core/Timer.h"
#include "plugins/ChatterBox.h"
#include "plugins/Diagnosis.h"
#include "plugins/ForceCommander.h"
#include "plugins/Governor.h"
#include "plugins/Miner.h"
#include "plugins/RepairMan.h"
#include "plugins/QuarterMaster.h"
#include "plugins/Scouting.h"
#include "plugins/WarpSmith.h"
#include "Reasoner.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <memory>

Dispatcher::Dispatcher(const std::string& opponent_id_): m_builder(new Builder()) {
    gAPI = std::make_unique<API::Interface>(Actions(), Control(), Debug(), Observation(), Query());
    gReasoner = std::make_unique<Reasoner>();
    gBrain = std::make_unique<Brain>();
    m_plugins.reserve(10);

    if (opponent_id_.empty())
        return;

    gHistory.info() << "Playing against an opponent with id "
        << opponent_id_ << std::endl;
}

void Dispatcher::OnGameStart() {
    m_plugins.clear();
    gHistory.info() << "New game started!" << std::endl;

    sc2::Race current_race = gAPI->observer().GetCurrentRace();
    gHub = std::make_unique<Hub>(current_race, CalculateExpansionLocations());

    m_plugins.emplace_back(new Governor());
    m_plugins.emplace_back(new Miner());
    m_plugins.emplace_back(new QuarterMaster());
    m_plugins.emplace_back(new RepairMan());
    m_plugins.emplace_back(new ForceCommander());
    m_plugins.emplace_back(new ChatterBox());
    m_plugins.emplace_back(new Scouting());

    if (current_race == sc2::Race::Protoss)
        m_plugins.emplace_back(new WarpSmith());

#ifdef DEBUG
    m_plugins.emplace_back(new Diagnosis());
#endif

    for (const auto& i : m_plugins)
        i->OnGameStart(m_builder.get());
}

void Dispatcher::OnGameEnd() {
    gHistory.info() << "Game over!" <<std::endl;

    for (const auto& i : m_plugins)
        i->OnGameEnd();
}

void Dispatcher::OnBuildingConstructionComplete(const sc2::Unit* building_) {
    gHistory.info() << sc2::UnitTypeToName(building_->unit_type) <<
        ": construction complete" << std::endl;

    gHub->OnBuildingConstructionComplete(*building_);

    for (auto& plugin : m_plugins)
        plugin->OnBuildingConstructionComplete(building_);
}

void Dispatcher::OnStep() {
    Timer clock;
    clock.Start();

    gHub->OnStep();
    gReasoner->CalculatePlayStyle();

    for (const auto& i : m_plugins)
        i->OnStep(m_builder.get());

    m_builder->OnStep();

    auto duration = clock.Finish();
    // 60ms is disqualification threshold of the ladder
    if (duration > 60.0f)
        gHistory.error() << "Step processing took: " << duration << " ms" << std::endl;

    // 44.4ms is highest allowed step time by the ladder
    if (duration > 44.4f)
        gHistory.warning() << "Step processing took: " << duration << " ms" << std::endl;
}

void Dispatcher::OnUnitCreated(const sc2::Unit* unit_) {
    // NOTE (alkurbatov): Could be just a worker exiting a refinery.
    if (unit_->alliance != sc2::Unit::Alliance::Self || IsGasWorker()(*unit_))
        return;

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
        " was created" << std::endl;

    gHub->OnUnitCreated(*unit_);

    for (const auto& i : m_plugins)
        i->OnUnitCreated(unit_);
}

void Dispatcher::OnUnitIdle(const sc2::Unit* unit_) {
    gHub->OnUnitIdle(*unit_);

    for (const auto& i : m_plugins)
        i->OnUnitIdle(unit_, m_builder.get());
}

void Dispatcher::OnUnitDestroyed(const sc2::Unit* unit_) {
    if (unit_->alliance != sc2::Unit::Alliance::Self)
        return;

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
        " was destroyed" << std::endl;

    gHub->OnUnitDestroyed(*unit_);

    for (const auto& i : m_plugins)
        i->OnUnitDestroyed(unit_, m_builder.get());
}

void Dispatcher::OnUpgradeCompleted(sc2::UpgradeID id_) {
    gHistory.info() << sc2::UpgradeIDToName(id_) << " completed (upgrade id: " <<
        id_ << ", ability id: " << gAPI->observer().GetUpgradeData(id_).ability_id << ")" << std::endl;

    for (const auto& i : m_plugins)
        i->OnUpgradeCompleted(id_);
}

void Dispatcher::OnUnitEnterVision(const sc2::Unit* unit) {
    for (const auto& i : m_plugins)
        i->OnUnitEnterVision(unit);
}

void Dispatcher::OnError(const std::vector<sc2::ClientError>& client_errors,
        const std::vector<std::string>& protocol_errors) {
    for (const auto i : client_errors) {
        gHistory.error() << "Encountered client error: " <<
            static_cast<int>(i) << std::endl;
    }

    for (const auto& i : protocol_errors)
        gHistory.error() << "Encountered protocol error: " << i << std::endl;
}
