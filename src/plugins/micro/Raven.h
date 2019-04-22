#pragma once

#include "MicroPlugin.h"

class Raven : public MicroPlugin {
public:
    explicit Raven(Unit* unit);

    void OnCombatStep(const Units& enemies) override;
};
