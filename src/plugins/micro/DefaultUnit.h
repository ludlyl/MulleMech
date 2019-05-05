#pragma once

#include "MicroPlugin.h"

class DefaultUnit : public MicroPlugin {
public:
    explicit DefaultUnit(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;
};
