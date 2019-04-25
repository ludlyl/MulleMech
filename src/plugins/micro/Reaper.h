#pragma once

#include "MicroPlugin.h"


class Reaper : public MicroPlugin {
public:
    explicit Reaper(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;
};
