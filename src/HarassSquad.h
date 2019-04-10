#pragma once

#include "Squad.h"
#include <sc2api/sc2_common.h>

class HarassSquad : public Squad {
public:
    HarassSquad() : m_sent(false) { }
    
    bool IsTaskFinished() const override;

    void Send();

    bool IsSent() const { return m_sent; }

protected:
    void Update() override;

    std::string SquadName() const override { return "HarassSquad (Id: #" + std::to_string(GetId()) + ")"; }

private:
    sc2::Point2D NextHarassTarget(bool first) const;

    bool m_sent;

    static constexpr float AggroRadius = 15.0f;             // attack workers that are within this distance
};
