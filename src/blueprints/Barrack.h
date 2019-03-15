#pragma once

#include "Building.h"

class Barrack : public Blueprint {
public:
    bool Build(Order* order) final;
};
