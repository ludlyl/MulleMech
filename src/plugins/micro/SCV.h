#pragma once

#include "MicroPlugin.h"

class SCV : public MicroPlugin {
public:
    explicit SCV(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

private:
    static constexpr int ActiveAutoRepairUnitValueThreshold = 1500;
};
