#include "OffenseSquad.h"

#include "Historican.h"
#include "core/API.h"
#include "core/Helpers.h"

bool OffenseSquad::IsFinished() const {
    return m_finished;
}

void OffenseSquad::TakeOver(const sc2::Point2D& position) {
    gHistory.info(LogChannel::combat) << SquadName() << " taking over new position" << std::endl;
    m_finished = false;
    m_attackPosition = position;
}

void OffenseSquad::Update() {
    if (m_finished)
        return;

    // Drop enemies that have gone too far away
    auto itr = std::remove_if(GetEnemies().begin(), GetEnemies().end(), [this](auto* u) {
        return Distance2D(GetCenter(), u->pos) >= MaxAttackRadius;
    });
    GetEnemies().erase(itr, GetEnemies().end());

    // Add enemies which are in combat range
    auto potentialEnemies = gAPI->observer().GetUnits(IsWithinDist(GetCenter(), AggroRadius), sc2::Unit::Alliance::Enemy);
    for (auto& enemy : potentialEnemies)
        if (!GetEnemies().contains(enemy))
            GetEnemies().push_back(enemy);

    if (GetEnemies().empty()) {
        // Approach attack position until we're in take over distance
        if (Distance2D(GetCenter(), m_attackPosition) < TakeOverRadius) {
            gHistory.debug(LogChannel::combat) << SquadName() << " took over region" << std::endl;
            m_finished = true;
            for (auto& unit : GetUnits())
                unit->Micro()->OnCombatOver(unit);
        } else if (!IsMoving()) {
            gHistory.debug(LogChannel::combat) << SquadName() << " approaching attack position" << std::endl;
            Approach(m_attackPosition);
        }
    } else {
        AbortMovement();
        // Ask micro plugin to deal with any enemies we have
        for (auto& unit : GetUnits())
            unit->Micro()->OnCombatFrame(unit, GetEnemies());
    }
}
