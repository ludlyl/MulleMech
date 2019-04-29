#pragma once

#include "Squad.h"
#include <sc2api/sc2_common.h>
#include <memory>

class ReinforceSquad : public Squad {
public:
    ReinforceSquad(std::shared_ptr<Squad> squad);

    bool IsTaskFinished() const override;

    // Send squad out to reinforce
    void Send();

    bool IsSent() const { return m_sent; }

protected:
    void Update() override;

    std::string SquadName() const override { return "ReinforceSquad (Id: #" + std::to_string(GetId()) + ")"; }

private:
    std::shared_ptr<Squad> m_targetSquad;
    bool m_sent;

    static constexpr float MaxAttackRadius = 16.0f;
    static constexpr float AggroRadius = 8.0f;
};
