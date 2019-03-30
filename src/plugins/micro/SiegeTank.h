#pragma once

#include "DefaultUnit.h"

class SiegeTank : public DefaultUnit {
public:
    SiegeTank(Unit* unit);

    void OnCombatStep(const Units& enemies) override;
};
