#include "DefenseSquad.h"
#include "Historican.h"

DefenseSquad::DefenseSquad(Units units, Units enemies) : m_engaged(false) {
    SetUnits(std::move(units));
    SetEnemies(std::move(enemies));
    gHistory.info(LogChannel::combat) << "New Squad: " << SquadName() << " formed (#units: " << GetUnits().size() <<
        ", #enemies: " << GetEnemies().size() << ")" << std::endl;
}

bool DefenseSquad::IsTaskFinished() const {
    return GetEnemies().empty() || GetUnits().empty();
}

bool DefenseSquad::UpdateEnemies(const Units& enemies) {
    for (auto& enemy : enemies) {
        if (GetEnemies().contains(enemy)) {
            SetEnemies(enemies);
            return true;
        }
    }

    return false;
}

void DefenseSquad::Update() {
    if (IsTaskFinished())
        return;

    if (m_engaged) {
        // Is combat over?
        if (IsTaskFinished()) {
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
        auto enemyCircle = GetEnemies().CalculateCircle();
        if (Distance2D(GetCenter(), enemyCircle.first) < EngageRadius + enemyCircle.second) {
            gHistory.debug(LogChannel::combat) << SquadName() << " engaging enemies" << std::endl;
            AbortMovement();
            m_engaged = true;
        } else if (!IsMoving()) {
            gHistory.debug(LogChannel::combat) << SquadName() << " approaching enemies" << std::endl;
            Approach(enemyCircle.first);
        }
    }
}
