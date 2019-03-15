#pragma once

#include "Building.h"

class Starport : public Blueprint {
public:
    bool Build(Order* order) final;
};
