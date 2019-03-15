#pragma once

#include "Building.h"

class Factory : public Blueprint {
public:
    bool Build(Order* order) final;
};
