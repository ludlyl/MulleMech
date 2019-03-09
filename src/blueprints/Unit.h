// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Blueprint.h"

struct Unit: Blueprint {
    explicit Unit(sc2::UNIT_TYPEID who_builds_, std::optional<sc2::UNIT_TYPEID> required_addon_ = std::nullopt);

    bool Build(Order* order_) final;

 private:
    sc2::UNIT_TYPEID m_who_builds;
    std::optional<sc2::UNIT_TYPEID> m_required_addon;
};
