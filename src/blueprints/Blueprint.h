// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Order.h"

#include <sc2api/sc2_typeenums.h>

#include <memory>

namespace bp {

struct Blueprint {
    virtual ~Blueprint();

    virtual bool Build(Order* order_) = 0;

    static std::shared_ptr<Blueprint> Plot(sc2::ABILITY_ID ability_);
};

}
