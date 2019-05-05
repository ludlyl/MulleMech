#include "HarassSquad.h"

#include "BuildingPlacer.h"
#include "Historican.h"
#include "IntelligenceHolder.h"
#include "Reasoner.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "Hub.h"

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
            unit->Micro()->OnCombatFrame(unit, GetEnemies(), GetUnits(), GetAttackMovePoint());
    }
}

sc2::Point2D HarassSquad::NextHarassTarget(bool first) const {
    sc2::Point3D target;

    // First point: we chance on a likely future expansion of our enemy
    if (first) {
        auto expansions = gReasoner->GetLikelyEnemyExpansions();
        if (!expansions.empty()) {
            if (expansions.size() >= 2 && sc2::GetRandomInteger(0, 1)) // 50% to check the second most likely
                target = expansions[1]->center_behind_minerals;
            else
                target = expansions.front()->center_behind_minerals;
        } else {
            return GetCenter(); // Idle as failure outcome
        }
    }
    // Else: we go to a known base location
    else {
        auto known_enemy_expansions = gIntelligenceHolder->GetKnownEnemyExpansions();
        if (!known_enemy_expansions.empty()) {
            // 50% chance to use latest base; otherwise pick random one
            if (sc2::GetRandomInteger(0, 1))
                target = known_enemy_expansions.back()->center_behind_minerals;
            else {
                target = known_enemy_expansions.at(
                        static_cast<Expansions::size_type>(sc2::GetRandomInteger(0, static_cast<int>(known_enemy_expansions.size()) - 1)))->center_behind_minerals;
            }
        } else {
            return GetCenter(); // Idle as failure outcome
        }
    }

    return target;
}
