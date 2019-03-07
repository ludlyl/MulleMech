// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Mutation.h"
#include "core/API.h"
#include "core/Helpers.h"

bool Mutation::Build(Order* order_) {
    auto targets = gAPI->observer().GetUnits(
        IsIdleUnit(order_->tech_alias.back()), sc2::Unit::Alliance::Self);

    if (targets().empty())
        return false;

    order_->assignee = targets().front()->tag;
    gAPI->action().Build(*order_);

    return true;
}
