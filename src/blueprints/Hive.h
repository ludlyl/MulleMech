// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Blueprint.h"

struct Hive: Blueprint {
    Hive();

    bool Build(Order* order_) final;
};