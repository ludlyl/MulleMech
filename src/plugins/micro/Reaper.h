#pragma once

#include "DefaultUnit.h"


class Reaper : public MicroPlugin {
public:
    explicit Reaper(Unit* unit);

    void OnCombatStep(const Units& enemies) override;
};
