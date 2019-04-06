#pragma once

#include "DefaultUnit.h"


class Reaper : public DefaultUnit {
public:
    Reaper(Unit* unit);

    void OnCombatStep(const Units& enemies) override;
};
