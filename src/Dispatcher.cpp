// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Dispatcher.h"
#include "Historican.h"
#include "Hub.h"
#include "Reasoner.h"
#include "IntelligenceHolder.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Map.h"
#include "core/Timer.h"
#include "plugins/ChatterBox.h"
#include "plugins/Diagnosis.h"
#include "plugins/CombatCommander.h"
#include "plugins/Governor.h"
#include "plugins/Miner.h"
#include "plugins/RepairMan.h"
#include "plugins/QuarterMaster.h"
#include "plugins/Scouting.h"
#include "BuildingPlacer.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <memory>

Dispatcher::Dispatcher(const std::string& opponent_id_): m_builder(new Builder()) {
    gAPI = std::make_unique<API::Interface>(Actions(), Control(), Debug(), Observation(), Query());
    gReasoner = std::make_unique<Reasoner>();
    gIntelligenceHolder = std::make_unique<IntelligenceHolder>();
    gBuildingPlacer = std::make_unique<BuildingPlacer>();
    m_plugins.reserve(10);

    if (opponent_id_.empty())
        return;

    gHistory.info() << "Playing against an opponent with id "
        << opponent_id_ << std::endl;
}

void Dispatcher::OnGameStart() {
    m_plugins.clear();
    gHistory.info() << "New game started!" << std::endl;

    gAPI->Init();

    sc2::Race current_race = gAPI->observer().GetCurrentRace();

    Timer clock;
    clock.Start();
    gHub = std::make_unique<Hub>(current_race, CalculateExpansionLocations());
    auto duration = clock.Finish();
    gHistory.info() << "Calculate Expansions took: " << duration << " ms" << std::endl;

    gOverseerMap = std::make_unique<Overseer::MapImpl>();

    clock.Start();
    gOverseerMap->setBot(this);
    gOverseerMap->initialize();
    gBuildingPlacer->OnGameStart();
    duration = clock.Finish();
    gHistory.info() << "Map calculations took: " << duration << " ms" << std::endl;
    gHistory.info() << "Tiles in start region: " << gOverseerMap->getNearestRegion(gAPI->observer().StartingLocation())->getTilePositions().size() << std::endl;

    gIntelligenceHolder->OnGameStart();

    m_plugins.emplace_back(new Governor());
    m_plugins.emplace_back(new Miner());
    m_plugins.emplace_back(new QuarterMaster());
    m_plugins.emplace_back(new RepairMan());
    m_plugins.emplace_back(new CombatCommander());
    m_plugins.emplace_back(new ChatterBox());
    m_plugins.emplace_back(new Scouting());

#ifdef DEBUG
    m_plugins.emplace_back(new Diagnosis());
#endif

    for (const auto& i : m_plugins)
        i->OnGameStart(m_builder.get());
}

void Dispatcher::OnGameEnd() {
    for (const auto& i : m_plugins)
        i->OnGameEnd();

    gAPI->control().SaveReplay();

    // NOTE: Do not change output without modifying the test script "testing.py"
    auto results = gAPI->observer().GetResults();
    if (results.size() == 2 && results[0].result == sc2::Win && results[1].result == sc2::Loss) {
        gHistory.info() << "Game over! Winning player id: {" << results[0].player_id << "}" << std::endl;
    } else if (results.size() == 2 && results[0].result == sc2::Loss && results[1].result == sc2::Win) {
        gHistory.info() << "Game over! Winning player id: {" << results[1].player_id << "}" << std::endl;
    } else {
        gHistory.info() << "Game over! No winner!" << std::endl;
    }
}

void Dispatcher::OnBuildingConstructionComplete(const sc2::Unit* building_) {
    gHistory.info() << sc2::UnitTypeToName(building_->unit_type) <<
        ": construction complete" << std::endl;

    auto building = gAPI->WrapAndUpdateUnit(building_);

    for (auto& plugin : m_plugins)
        plugin->OnBuildingConstructionComplete(building);
}

void Dispatcher::OnStep() {
    Timer clock;
    clock.Start();

    gAPI->OnStep();
    gReasoner->CalculatePlayStyle();
    gReasoner->CalculateNeededUnitClasses();

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
    auto unit = gAPI->WrapAndUpdateUnit(unit_);
    // NOTE (alkurbatov): Could be just a worker exiting a refinery.
    if (unit_->alliance != sc2::Unit::Alliance::Self || IsWorkerWithJob(Worker::Job::gathering_vespene)(unit))
        return;

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) <<
        " was created" << std::endl;

    gHub->OnUnitCreated(unit);
    m_builder->OnUnitCreated(unit);
    gBuildingPlacer->OnUnitCreated(unit);

    for (const auto& i : m_plugins)
        i->OnUnitCreated(unit);
}

void Dispatcher::OnUnitIdle(const sc2::Unit* unit_) {
    auto unit = gAPI->WrapAndUpdateUnit(unit_);
    m_builder->OnUnitIdle(unit);

    for (const auto& i : m_plugins)
        i->OnUnitIdle(unit, m_builder.get());
}

void Dispatcher::OnUnitDestroyed(const sc2::Unit* unit_) {
    // The documentation for this function says that it's only called for our own units, but that ins't the case

    auto unit = gAPI->WrapAndUpdateUnit(unit_);
    gHub->OnUnitDestroyed(unit);
    gBuildingPlacer->OnUnitDestroyed(unit);
    gIntelligenceHolder->OnUnitDestroyed(unit);

    // TODO: This check shouldn't have to be done here. Make sure Hub and all plugins check alliance themselves
    if (unit_->alliance == sc2::Unit::Alliance::Self) {
        gHistory.info() << "One of our: " << sc2::UnitTypeToName(unit_->unit_type) << " was destroyed" << std::endl;

        m_builder->OnUnitDestroyed(unit);
        gHub->OnUnitDestroyed(unit);
        for (const auto& i : m_plugins)
            i->OnUnitDestroyed(unit, m_builder.get());
    }
}

void Dispatcher::OnUpgradeCompleted(sc2::UpgradeID id_) {
    gHistory.info() << sc2::UpgradeIDToName(id_) << " completed (upgrade id: " <<
        id_ << ", ability id: " << gAPI->observer().GetUpgradeData(id_).ability_id << ")" << std::endl;

    gAPI->OnUpgradeCompleted();

    for (const auto& i : m_plugins)
        i->OnUpgradeCompleted(id_);
}

void Dispatcher::OnUnitEnterVision(const sc2::Unit* unit_) {
    auto unit = gAPI->WrapAndUpdateUnit(unit_);
    gBuildingPlacer->OnUnitEnterVision(unit);
    gIntelligenceHolder->OnUnitEnterVision(unit);
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
