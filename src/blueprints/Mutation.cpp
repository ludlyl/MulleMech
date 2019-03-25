// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Mutation.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "../Hub.h"

bool bp::Mutation::Build(Order* order_) {
    if (gHub->AssignBuildingProduction(order_, order_->tech_alias.back())) {
        gAPI->action().Build(*order_);
        return true;
    }

    return false;
}
