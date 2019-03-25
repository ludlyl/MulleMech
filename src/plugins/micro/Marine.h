#pragma once

#include "DefaultUnit.h"

class Marine : public DefaultUnit {
public:
    Marine(const Unit& unit);

    void OnCombatStep(const Units& enemies) override;

    static constexpr float StimHpPct = 0.7f;
};
