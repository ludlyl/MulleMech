#pragma once

#include "Building.h"

class SupplyDepot : public Blueprint {
public:
    bool Build(Order* order) final;
};
