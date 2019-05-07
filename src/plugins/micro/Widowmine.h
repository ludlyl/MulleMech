#pragma once

#include "MicroPlugin.h"

class Widowmine : public MicroPlugin {
public:
    explicit Widowmine(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

private:
    static constexpr float BurrowDownRange  = 8.0f; // Actual range 5: burrow a bit preemptively
    static constexpr float BurrowUpRange    = 11.0f;
};
