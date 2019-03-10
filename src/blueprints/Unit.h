// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Blueprint.h"

struct Unit: Blueprint {
    // Here if required_addon_ = INVALID it means the add-on doesn't matter
    explicit Unit(sc2::UNIT_TYPEID who_builds_, sc2::UNIT_TYPEID required_addon_ = sc2::UNIT_TYPEID::INVALID);

    bool Build(Order* order_) final;

 private:
    sc2::UNIT_TYPEID m_who_builds;
    sc2::UNIT_TYPEID m_required_addon;
};
