// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include <sc2api/sc2_data.h>

class Unit;

struct Order {
    explicit Order(const sc2::UnitTypeData& data_, Unit* assignee_ = nullptr);

    explicit Order(const sc2::UpgradeData& data_);

    std::string name;

    int mineral_cost;
    int vespene_cost;

    float food_required;
    std::vector<sc2::UnitTypeID> tech_requirements;

    sc2::UnitTypeID unit_type_id;
    sc2::AbilityID ability_id;
    std::vector<sc2::UnitTypeID> tech_alias;

    Unit* assignee = nullptr;
};
