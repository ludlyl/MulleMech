// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Diagnosis.h"
#include "Hub.h"
#include "Historican.h"
#include "core/API.h"

void Diagnosis::OnStep(Builder* builder_) {
    gAPI->debug().DrawText("Nonsequential build order:");

    if (builder_->GetNonsequentialConstructionOrders().empty()) {
        gAPI->debug().DrawText("Empty");
    } else {
        for (const auto& i : builder_->GetNonsequentialConstructionOrders())
            gAPI->debug().DrawText(i.name);
    }

    gAPI->debug().DrawText("_______________________");
    gAPI->debug().DrawText("Sequential build order:");

    if (builder_->GetSequentialConstructionOrders().empty()) {
        gAPI->debug().DrawText("Empty");
    } else {
        for (const auto& i : builder_->GetSequentialConstructionOrders())
            gAPI->debug().DrawText(i.name);
    }

    for (const auto& i : gHub->GetExpansions())
        gAPI->debug().DrawSphere(i->town_hall_location, 0.35f);

    gAPI->debug().SendDebug();
}

void Diagnosis::OnGameEnd() {
    gAPI->control().SaveReplay();
}
