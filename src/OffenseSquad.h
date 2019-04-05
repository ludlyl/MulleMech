#pragma once

#include "Squad.h"
#include <sc2api/sc2_common.h>

class OffenseSquad : public Squad {
public:
    OffenseSquad() : m_finished(true), m_defending(false) { }

    bool IsTaskFinished() const override;

    void TakeOver(sc2::Point2D position);

    void AbortTakeOver() { m_finished = true; m_defending = false; }

    bool HasTask();

protected:

    void Update() override;

    std::string SquadName() const override { return "OffenseSquad (Id: #" + std::to_string(GetId()) + ")"; }

private:
    sc2::Point2D m_attackPosition;
    bool m_finished;
    bool m_defending;
    bool m_hasTask;

    static constexpr float TakeOverRadius = 10.0f;          // to determine if we're close enough to our target pos
    static constexpr float AggroRadius = 10.0f;             // attack enemies that are within this distance
    static constexpr float MaxAttackRadius = 20.0f;         // drop enemies that went this far away
};
