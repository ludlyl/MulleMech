#include "HarassSquad.h"

#include "BuildingPlacer.h"
#include "Historican.h"
#include "IntelligenceHolder.h"
#include "Reasoner.h"
#include "core/API.h"
#include "core/Helpers.h"

bool HarassSquad::IsTaskFinished() const {
    return GetUnits().empty(); // We harass 'til death!
}

void HarassSquad::Send() {
    gHistory.info(LogChannel::combat) << SquadName() << " heading out to harass!" << std::endl;
    m_sent = true;
    Approach(NextHarassTarget(true));
}

void HarassSquad::Update() {
    if (!m_sent)
        return;

    if (IsTaskFinished()) {
        AbortMovement();
        m_sent = false;
        return; // Reset
    }

    auto workers = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
        {IsWorker(), IsWithinDist(GetCenter(), AggroRadius)}), sc2::Unit::Alliance::Enemy);

    if (workers.empty() && IsMoving())
        return;

    if (workers.empty()) {
        for (auto& unit : GetUnits())
            unit->Micro()->OnCombatOver(unit);
        Approach(NextHarassTarget(false));
    } else {
        SetEnemies(std::move(workers));
        for (auto& unit : GetUnits())
            unit->Micro()->OnCombatFrame(unit, GetEnemies());
    }
}

sc2::Point2D HarassSquad::NextHarassTarget(bool first) const {
    sc2::Point3D base;

    // First point: we chance on a likely future expansion of our enemy
    if (first) {
        auto expansions = gReasoner->GetLikelyEnemyExpansions();
        if (!expansions.empty()) {
            base = expansions.front()->town_hall_location;
            if (expansions.size() >= 2 && sc2::GetRandomInteger(0, 1)) // 50% to check the second most likely
                base = expansions[1]->town_hall_location;
        } else {
            return GetCenter(); // Idle as failure outcome
        }
    }
    // Else: we go to a known base location
    else {
        if (gIntelligenceHolder->GetEnemyBaseCount() == 0) {
            auto start = gAPI->observer().GameInfo().enemy_start_locations.front();
            base = sc2::Point3D(start.x, start.y, 0);
        } else {
            // 50% chance to use latest base; otherwise pick random one
            if (sc2::GetRandomInteger(0, 1))
                base = gIntelligenceHolder->GetLatestEnemyBase()->town_hall_location;
            else
                base = gIntelligenceHolder->GetEnemyBase(sc2::GetRandomInteger(0, gIntelligenceHolder->GetEnemyBaseCount() - 1))->town_hall_location;
        }
    }
    
    // Find location behind mineral line of expansion
    return BuildingPlacer::GetCenterBehindMinerals(base);
}
