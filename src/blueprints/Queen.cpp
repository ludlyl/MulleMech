// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Queen.h"
#include "core/API.h"
#include "core/Helpers.h"

bool Queen::Build(Order* order_) {
    auto town_halls = gAPI->observer().GetUnits(IsIdleTownHall(), sc2::Unit::Alliance::Self);
    if (town_halls().empty())
        return false;

    order_->assignee = town_halls().front()->tag;

    gAPI->action().Build(*order_);

    return true;
}
