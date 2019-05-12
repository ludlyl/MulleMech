// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "RepairMan.h"
#include "core/API.h"
#include "core/Helpers.h"

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

    Units buildings = gAPI->observer().GetUnits(IsBuilding(), sc2::Unit::Alliance::Self);

    for (Unit* unit : buildings) {
        // i.e only handle fully constructed buildings/workers
        if (unit->build_progress < 1.0f) {
            continue;
        }

        //TODO: more cases
        /*if (littleHP) {
            //do this
        }
        if (mediumMuch) {
            //do this
        }
        if (alot) {
            //do this
        }*/

        // i.e repair Planetary fortress with all close mineral harvesting workers
        if (unit->health < 1500 && IsPlanetaryFortress()(*unit) && unit->m_repairPhase == Unit::BuildingRepairPhase::not_repairing) {
            auto workers = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                                 {IsWithinDist(unit->pos, 15.0f), IsHasrvestingMineralsWorker()}));
            if (!workers.empty()) {
                unit->m_repairPhase = Unit::BuildingRepairPhase::repairing;
                for (Unit* worker : workers) {
                    //if (FreeWorkerExists()) {
                    worker->AsWorker()->SetAsRepairer(unit);
                }
            }
        }

        // repair Orbital Command (on ground or flying) if it is dropping health
        if (unit->health < 1499 && IsOrbitalCommand()(*unit) && unit->m_repairPhase == Unit::BuildingRepairPhase::not_repairing) {
            unit->m_repairPhase = Unit::BuildingRepairPhase::repairing;
            repairMen = 3;
            for (int i = 0; i < repairMen; i++) {
                if (!FreeWorkerExists()) {
                    break;
                }
                auto worker = GetClosestFreeWorker(unit->pos);
                worker->AsWorker()->SetAsRepairer(unit);
            }
        }
    }

    for (Unit* unit : buildings) {
        // checks if building has full health after repairing
        if (unit->m_repairPhase == Unit::BuildingRepairPhase::repairing && unit->health + std::numeric_limits<float>::epsilon() >= unit->health_max) {
            // Get close repair workers
            auto workers = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                                 {IsWithinDist(unit->pos, 15.0f), IsRepairWorker()}));
            // Release workers if there is any
            if (!workers.empty()) {
                for (Unit *repairer : workers) {
                    repairer->AsWorker()->SetAsUnemployed();
                }
            }
            // Set building as not repairing
            unit->m_repairPhase = Unit::BuildingRepairPhase::not_repairing;
        }

        //if somehow there is buildings in repair-mode but no repairers
        if (unit->m_repairPhase == Unit::BuildingRepairPhase::repairing && ((gAPI->observer().GetUnits(IsRepairWorker())).size()) < 1) {
            unit->m_repairPhase = Unit::BuildingRepairPhase::not_repairing;
        }

        // i.e handle idle repairers
        Units repairers = gAPI->observer().GetUnits(IsRepairWorker());
        if (!repairers.empty() && unit->m_repairPhase == Unit::BuildingRepairPhase::repairing) {
            for (Unit *repairer : repairers) {
                if (repairer->IsIdle()) {
                    repairer->AsWorker()->SetAsRepairer(unit);
                }
            }
        }
    }

    // recover if repairers mystically stops repairing but still are repairer for a certain building
    for (Unit* unit : buildings) {
        for (Unit* repairer : (gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                                     {IsWithinDist(unit->pos, 15.0f), IsRepairWorker()})))) {
            if (unit->health < unit->health_max && IsOrbitalCommand()(*unit)) {
                repairer->AsWorker()->SetAsRepairer(unit);
            }
        }
    }

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

std::size_t RepairMan::CountRepairMen(Unit* unit) {
    std::size_t nOfRepairMen = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                                     {IsWithinDist(unit->pos, 15.0f), IsRepairWorker()})).size();
    return nOfRepairMen;
}

bool RepairMan::IsAnyRepairersNearby(Unit* unit) {
    std::size_t nOfRepairMen = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                                     {IsWithinDist(unit->pos, 15.0f), IsRepairWorker()})).size();
    if (nOfRepairMen > 0) {
        return true;
    }
    return false;
}
