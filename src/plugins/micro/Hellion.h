#pragma once

#include "MicroPlugin.h"

class Hellion : public MicroPlugin {
public:
    Hellion(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;
};
