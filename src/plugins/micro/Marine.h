#pragma once

#include "DefaultUnit.h"

class Marine : public DefaultUnit {
public:
    explicit Marine(Unit* unit);

    void OnCombatStep(const Units& enemies, const Units& allies) override;

    static constexpr float StimHpPct = 0.7f;
};
