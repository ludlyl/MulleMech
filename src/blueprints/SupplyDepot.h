#pragma once

#include "Building.h"
#include <optional>

namespace bp {

class SupplyDepot : public Blueprint {
public:
    bool CanBeBuilt(const Order* order) final;

    bool Build(Order* order) final;

private:
    std::optional<sc2::Point3D> FindBuildLocation(const Order* order_) const;
};

}
