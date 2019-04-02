#pragma once

#include "Blueprint.h"

namespace bp {

// I.e. for handling barracks, factories & starports
class ProductionBuilding : public Blueprint {
public:
    bool Build(Order *order) final;
};

}
