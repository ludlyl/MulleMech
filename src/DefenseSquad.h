#pragma once

#include "Squad.h"

class DefenseSquad : public Squad {
public:
    DefenseSquad(Units units, Units enemies);

    bool IsFinished() const override;

protected:
    void Update() override;

    std::string SquadName() const override { return "DefenseSquad (Id: #" + std::to_string(GetId()) + ")"; }

private:
    bool m_engaged;

    static constexpr float EngageRadius = 15.0f; // move as a unit until we get this close, then engage
};
