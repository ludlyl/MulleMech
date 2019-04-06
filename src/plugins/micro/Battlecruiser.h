#pragma once

#include "DefaultUnit.h"

class Battlecruiser : public DefaultUnit {
public:
    Battlecruiser(Unit* unit);

    void OnCombatStep(const Units& enemies) override;

};
