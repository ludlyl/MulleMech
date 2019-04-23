// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Blueprint.h"

namespace bp {

// Could take an expansion as a parameter to support only (trying) to build refinery at a certain expansion
struct Refinery : Blueprint {
    bool CanBeBuilt(const Order* order_) final;

    bool Build(Order* order_) final;
};

}
