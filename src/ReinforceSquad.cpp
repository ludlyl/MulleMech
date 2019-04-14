#include "ReinforceSquad.h"

#include "Historican.h"
#include "core/Helpers.h"

ReinforceSquad::ReinforceSquad(std::shared_ptr<Squad> squad) : m_targetSquad(std::move(squad)), m_sent(false) { }

bool ReinforceSquad::IsTaskFinished() const {
    return m_targetSquad->GetUnits().empty() ||
        Distance2D(m_targetSquad->GetCenter(), GetCenter()) <= m_targetSquad->GetSpreadRadius();
}

void ReinforceSquad::Send() {
    gHistory.info(LogChannel::combat) << SquadName() << " heading out to reinforce!" << std::endl;
    m_sent = true;
}

void ReinforceSquad::Update() {
    if (!m_sent)
        return;

    // Drop enemies that have gone too far away
    auto itr = std::remove_if(GetEnemies().begin(), GetEnemies().end(), [this](auto* u) {
        return Distance2D(GetCenter(), u->pos) >= MaxAttackRadius;
    });
    GetEnemies().erase(itr, GetEnemies().end());

    // Add enemies which are in combat range
    auto potentialEnemies = gAPI->observer().GetUnits(IsWithinDist(GetCenter(), AggroRadius), sc2::Unit::Alliance::Enemy);
    for (auto& enemy : potentialEnemies) {
        if (!GetEnemies().contains(enemy))
            GetEnemies().push_back(enemy);
    }

    // If we have enemies attacking us => attack them
    if (!GetEnemies().empty()) {
        for (auto& unit : GetEnemies())
            unit->Micro()->OnCombatFrame(unit, GetEnemies(), GetAllies());
        return; // Don't do reinforce movement logic
    }

    if (!IsMoving() || Distance2D(m_targetSquad->GetCenter(), GetApproachPoint()) > m_targetSquad->GetSpreadRadius() / 2.0f) {
        Approach(m_targetSquad->GetCenter());
    }
}
