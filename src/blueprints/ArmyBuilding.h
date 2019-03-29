#pragma once

#include "Building.h"

class ArmyBuilding : public bp::Blueprint {
public:
    bool Build(Order* order) final;
};
