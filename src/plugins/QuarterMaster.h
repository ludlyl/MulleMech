// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct QuarterMaster : Plugin {
    QuarterMaster();

    void OnStep(Builder* builder_) final;

    void OnUnitCreated(Unit* unit_) final;

 private:
    bool m_skip_turn;
    static constexpr float m_expected_supply_margin_quotient = 1.2f;
};
