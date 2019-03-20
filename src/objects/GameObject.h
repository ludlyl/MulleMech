// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Unit.h"

#include <sc2api/sc2_gametypes.h>
#include <sc2api/sc2_unit.h>

struct GameObject {
    explicit GameObject(sc2::Tag tag_);

    explicit GameObject(const Unit& unit_);

    bool operator==(const GameObject& obj_) const;

    bool operator==(const Unit& unit_) const;

    sc2::Tag Tag() const;

    sc2::Point3D GetPos() const;

    Unit ToUnit() const;

    static Unit ToUnit(sc2::Tag tag_);

 private:
    sc2::Tag m_tag;
};
