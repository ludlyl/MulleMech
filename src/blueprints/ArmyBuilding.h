#pragma once

#include "Building.h"

class ArmyBuilding : public Blueprint {
public:
    bool Build(Order* order) final;
};
