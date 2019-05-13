// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "RepairMan.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Map.h"
#include "Hub.h"

#include <cmath>

void RepairMan::OnStep(Builder*) {
    // Cancel constructions under start hp
    // A optimization would be to save ongoing constructions instead
    // TODO: Maybe this should be made more intelligent? I.e. don't cancel a building if the threat is killed
    //  or the scv is building at a faster speed than it's losing hp, if the building very soon is finished,
    //  cancel it before this percentage if the enemy units risks killing it in 1 step
    Units ongoing_constructions = gAPI->observer().GetUnits(IsUnfinishedBuilding(), sc2::Unit::Alliance::Self);
    for (auto& building : ongoing_constructions) {
        if (building->health / building->health_max <= BuildingMinHealthCancelRatio) {
            gAPI->action().Cast(building, sc2::ABILITY_ID::CANCEL);
        }
    }

    // Reparation logic

    // Note: All finished terran buildings are mechanical (and thereby repairable)
    Units damaged_buildings = gAPI->observer().GetUnits(
            MultiFilter(MultiFilter::Selector::And, {IsFinishedBuilding(), IsDamaged()}), sc2::Unit::Alliance::Self);

    if (!damaged_buildings.empty()) {
        auto repairing_workers = gAPI->observer().GetUnits(IsWorkerWithJob(Worker::Job::repairing), sc2::Unit::Alliance::Self);
        // This is only needed as we do not cache the free workers. If we start to cache them this can be removed
        auto free_workers = GetFreeWorkers(true);

        // TODO: Do not send units to buildings with a lot of enemy units nearby
        //  (the scvs are currently just sent to their deaths)
        for (auto& building : damaged_buildings) {
            if (free_workers.empty()) {
                break;
            }

            int number_of_currently_repairing_scvs = CountRepairingScvs(repairing_workers, building);
            for (; number_of_currently_repairing_scvs < DefaultRepairingScvCount; number_of_currently_repairing_scvs++) {
                // Would probably be faster to just sort free_workers based on their distance
                // to the building at the start of the damaged_buildings loop and use pop_back
                const auto& worker = free_workers.GetClosestUnit(building->pos);
                if (worker) {
                    worker->AsWorker()->Repair(building);
                    free_workers.remove(worker);
                } else {
                    break;
                }
            }

            // Check if the building is a "combat building" (i.e. a turret, bunker or pf)
            if (!free_workers.empty() && IsCombatUnit()(building)) {
                int maximum_repair_count = GetMaximumScvRepairCountFor(free_workers.front(), building);

                if (number_of_currently_repairing_scvs < maximum_repair_count) {
                    const auto& closest_region = gOverseerMap->getNearestRegion(building->pos); // Is this costly?
                    const auto& closest_expansion = gHub->GetClosestExpansion(closest_region->getMidPoint());
                    if (closest_expansion->alliance == sc2::Unit::Alliance::Self) {
                        Units free_workers_with_expansion_as_home_base;
                        for (auto& worker : free_workers) {
                            if (worker->AsWorker()->GetHomeBase() == closest_expansion) {
                                free_workers_with_expansion_as_home_base.push_back(worker);
                            }
                        }

                        for (; number_of_currently_repairing_scvs < maximum_repair_count; number_of_currently_repairing_scvs++) {
                            // Do we "have" to the the closest unit in this scenario? Most workers will be pretty close anyway
                            const auto& worker = free_workers_with_expansion_as_home_base.GetClosestUnit(building->pos);
                            if (worker) {
                                worker->AsWorker()->Repair(building);
                                free_workers.remove(worker);
                                free_workers_with_expansion_as_home_base.remove(worker);
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void RepairMan::OnUnitIdle(Unit* unit_, Builder*) {
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_SCV) {
        if (unit_->AsWorker()->GetJob() == Worker::Job::repairing) {
            unit_->AsWorker()->SetAsUnemployed();
        }
    }
}

void RepairMan::OnUnitDestroyed(Unit* unit_, Builder* builder_) {
    if (!IsBuilding()(unit_))
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
            builder_->ScheduleSequentialConstruction(unit_->GetTypeData()->tech_alias.front(), true);    // Parent
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

int RepairMan::GetMaximumScvRepairCountFor(Unit* scv_, Unit* unit_) const {
    // TODO: Make this "support" flying units (i.e. implement some basic kind of circle packing)

    // The formula that is used to determine how many circles of radius r
    // that can be wrapped around a circle of radius R is:
    // pi / (arcsin(r/(r+R)))

    auto scv_radius = static_cast<double>(scv_->radius);
    auto unit_radius = static_cast<double>(unit_->radius);

    return static_cast<int>(std::floor(F_PI / (std::asin(scv_radius / (scv_radius + unit_radius)))));
}

int RepairMan::CountRepairingScvs(const Units& scvs, Unit* unit_) const {
    int count = 0;
    for (auto& scv : scvs) {
        if (!scv->GetPreviousStepOrders().empty() &&
            scv->GetPreviousStepOrders().front().ability_id == sc2::ABILITY_ID::EFFECT_REPAIR &&
            scv->GetPreviousStepOrders().front().target_unit_tag == unit_->tag) {
            count++;
        }
    }
    return count;
}
