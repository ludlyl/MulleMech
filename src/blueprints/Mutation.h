// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Blueprint.h"

namespace bp {

struct Mutation : Blueprint {
    bool CanBeBuilt(const Order* order_) final;

    bool Build(Order* order_) final;
};

}
