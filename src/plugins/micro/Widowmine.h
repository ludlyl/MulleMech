#pragma once

#include "MicroPlugin.h"

class Widowmine : public MicroPlugin {
public:
    explicit Widowmine(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    static constexpr float BurrowRange = 13;
};
