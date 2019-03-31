#pragma once

#include "DefaultUnit.h"

class Hellion : public DefaultUnit {
public:
    Hellion(Unit* unit);

    void OnCombatStep(const Units& enemies) override;
};
