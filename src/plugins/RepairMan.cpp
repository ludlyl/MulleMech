// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "RepairMan.h"
#include "core/API.h"
#include "core/Helpers.h"

void RepairMan::OnStep(Builder*) {
    if (gAPI->observer().GetCurrentRace() != sc2::Race::Terran)
        return;

    // FIXME (alkuratov): Put buildings repair code here.
}

void RepairMan::OnUnitDestroyed(Unit* unit_, Builder* builder_) {
    if (!IsBuilding()(*unit_))
        return;

    // Add upgrades that was researched by the building (or it's addon) back into the queue
    AddQueuedUpgradesBackIntoBuildingQueue(unit_, builder_);
    if (auto addon = unit_->GetAttachedAddon()) {
        AddQueuedUpgradesBackIntoBuildingQueue(addon, builder_);
    }

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_TECHLAB:
        case sc2::UNIT_TYPEID::TERRAN_REACTOR:
            // As we don't know how to rebuild "generic" add-ons we ignore them for now
            return;

        case sc2::UNIT_TYPEID::PROTOSS_PYLON:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED:
            // NOTE (alkurbatov): QuarterMaster is responsible for supplies rebuild.
            return;

        // Morphed buildings
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
            builder_->ScheduleSequentialConstruction(unit_->GetTypeData().tech_alias.front(), true);    // Parent
            builder_->ScheduleNonsequentialConstruction(unit_->unit_type.ToType());                     // Mutation
            return;

        default:
            // Schedule an addon if the building had one
            if (auto addon = unit_->GetAttachedAddon()) {
                // NOTE: The addon is not orphaned yet at this point, as such we can just reconstruct its type
                builder_->ScheduleNonsequentialConstruction(addon->unit_type);
            }

            // Schedule the building for reconstruction
            builder_->ScheduleConstructionInRecommendedQueue(unit_->unit_type.ToType(), true);
            return;
    }
}

void RepairMan::AddQueuedUpgradesBackIntoBuildingQueue(const Unit* unit_, Builder* builder_) const {
    for (const auto& order : unit_->GetPreviousStepOrders()) {
        // Should add a IsMutation in Helpers or something like that to avoid hard-coding in values like this...
        if (order.ability_id == sc2::ABILITY_ID::MORPH_ORBITALCOMMAND
            || order.ability_id == sc2::ABILITY_ID::MORPH_PLANETARYFORTRESS) {
            auto unit_id = gAPI->observer().GetUnitConstructedFromAbility(order.ability_id);
            builder_->ScheduleConstructionInRecommendedQueue(unit_id);
            return;
        }

        auto upgrade_id = gAPI->observer().GetUpgradeFromAbility(order.ability_id);
        if (upgrade_id != sc2::UPGRADE_ID::INVALID) {
            builder_->ScheduleUpgrade(upgrade_id);
        }
    }
}
