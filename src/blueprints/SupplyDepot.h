#pragma once

#include "Building.h"

namespace bp {

class SupplyDepot : public Blueprint {
public:
    bool CanBeBuilt(const Order* order) final;

    bool Build(Order* order) final;
};

}
