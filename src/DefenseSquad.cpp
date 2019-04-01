#include "DefenseSquad.h"
#include "Historican.h"

DefenseSquad::DefenseSquad(Units units, Units enemies) : m_engaged(false) {
    SetUnits(std::move(units));
    SetEnemies(std::move(enemies));
    gHistory.info(LogChannel::combat) << "New Squad: " << SquadName() << " formed (#units: " << GetUnits().size() <<
        ", #enemies: " << GetEnemies().size() << ")" << std::endl;
}

bool DefenseSquad::IsFinished() const {
    return GetEnemies().empty() || GetUnits().empty();
}

void DefenseSquad::Update() {
    if (IsFinished())
        return;

    if (m_engaged) {
        // Is combat over?
        if (IsFinished()) {
            gHistory.debug(LogChannel::combat) << SquadName() << " finished" << std::endl;
            for (auto& unit : GetUnits())
                unit->Micro()->OnCombatOver(unit);
            return;
        }

        // Let micro plugin dispose of enemy if we're engaged
        for (auto& unit : GetUnits())
            unit->Micro()->OnCombatFrame(unit, GetEnemies());
    } else {
        // Approach enemy if we've not engaged yet
        auto enemyCenter = GetEnemies().CalculateCircle().first;
        if (Distance2D(GetCenter(), enemyCenter) < EngageRadius) {
            gHistory.debug(LogChannel::combat) << SquadName() << " engaging enemies" << std::endl;
            AbortMovement();
            m_engaged = true;
        } else if (!IsMoving()) {
            gHistory.debug(LogChannel::combat) << SquadName() << " approaching enemies" << std::endl;
            Approach(enemyCenter);
        }
    }
}
