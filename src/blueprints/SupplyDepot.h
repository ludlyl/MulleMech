#pragma once

#include "Building.h"

namespace bp {

class SupplyDepot : public Blueprint {
public:
    bool Build(Order* order) final;
};

}
