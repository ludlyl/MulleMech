#pragma once

#include "MicroPlugin.h"

class Medivac : public MicroPlugin {
public:
    explicit Medivac(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    void OnCombatEnded() override { m_stopped = false; }

private:
    bool m_stopped = false;

    static constexpr float HealRange = 4.0f * 1.5f;
};
