#pragma once

#include "DefaultUnit.h"

class SiegeTankSieged : public DefaultUnit {
public:
    SiegeTankSieged (Unit* unit);

    void OnCombatStep(const Units& enemies) override;
};
