// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "RepairMan.h"
#include "core/API.h"
#include "core/Helpers.h"

void RepairMan::OnStep(Builder*) {
    if (gAPI->observer().GetCurrentRace() != sc2::Race::Terran)
        return;

    Units buildings = gAPI->observer().GetUnits(IsBuilding(), sc2::Unit::Alliance::Self);
    for (Unit* unit : buildings) {
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

        // i.e repair upgraded CC with all close mineral harvesting workers
        if (unit->health < 1500 && IsPlanetaryFortress()(*unit) && unit->m_repairPhase == Unit::BuildingRepairPhase::not_repairing) {
            unit->m_repairPhase = Unit::BuildingRepairPhase::repairing;
            auto workers = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                                 {IsWithinDist(unit->pos, 15.0f), IsHasrvestingMineralsWorker()}));
            for (Unit* worker : workers) {
                if (FreeWorkerExists()) {
                    worker->AsWorker()->SetAsRepairer(unit);
                }
            }
        }

        // repair Orbital Command (on ground or flying) if it is dropping health
        if (unit->health < 1400 && IsOrbitalCommand()(*unit) && unit->m_repairPhase == Unit::BuildingRepairPhase::not_repairing) {
            unit->m_repairPhase = Unit::BuildingRepairPhase::repairing;
            repairMen = 3;
            for (int i = 0; i < repairMen; i++) {
                if (FreeWorkerExists()) {
                    auto worker = GetClosestFreeWorker(unit->pos);
                    worker->SetAsRepairer(unit);
                }
            }
        }
    }

    // checks if building has full health after repairing
    for (Unit* unit : buildings) {
        if (unit->m_repairPhase == Unit::BuildingRepairPhase::repairing && unit->health >= 0.94f * unit->health_max) {
            // Get close repair workers
            auto workers = gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
                                                                 {IsWithinDist(unit->pos, 15.0f), IsRepairWorker()}));
            // Release workers
            for (Unit* repairer : workers) {
                repairer->AsWorker()->SetAsUnemployed();
            }
            // Set building as not repairing
            unit->m_repairPhase = Unit::BuildingRepairPhase::not_repairing;
        }
    }


    // FIXME (alkuratov): Put buildings repair code here.
}

void RepairMan::OnUnitDestroyed(Unit* unit_, Builder* builder_) {
    if (IsCombatUnit()(*unit_))
        return;

    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_TECHLAB:
        case sc2::UNIT_TYPEID::TERRAN_REACTOR:
            // As we don't know how to rebuild "generic" add-ons we ignore them for now
            return;

        case sc2::UNIT_TYPEID::PROTOSS_PYLON:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED:
        case sc2::UNIT_TYPEID::ZERG_OVERLORD:
        case sc2::UNIT_TYPEID::ZERG_OVERLORDCOCOON:
        case sc2::UNIT_TYPEID::ZERG_OVERLORDTRANSPORT:
        case sc2::UNIT_TYPEID::ZERG_OVERSEER:
            // NOTE (alkurbatov): QuarterMaster is responsible for supplies rebuild.
            return;

        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
        case sc2::UNIT_TYPEID::ZERG_DRONEBURROWED:
            // NOTE (alkurbatov): Miner is responsible for workers rebuild.
            return;

        case sc2::UNIT_TYPEID::TERRAN_MULE:
        case sc2::UNIT_TYPEID::TERRAN_POINTDEFENSEDRONE:
        case sc2::UNIT_TYPEID::TERRAN_KD8CHARGE:
        case sc2::UNIT_TYPEID::ZERG_BROODLING:
        case sc2::UNIT_TYPEID::ZERG_EGG:
        case sc2::UNIT_TYPEID::ZERG_LARVA:
            return;

        // Morphed buildings (TODO: Is there some generic way to find parent building of a morphed building)
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
            builder_->ScheduleSequentialConstruction(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER, true);   // Parent
            builder_->ScheduleNonsequentialConstruction(unit_->unit_type.ToType());                   // Mutation
            return;

        default:
            // Schedule an addon if the building had one
            if (auto addon = gAPI->observer().GetUnit(unit_->add_on_tag)) {
                // NOTE: The addon is not orphaned yet at this point, as such we can just reconstruct its type
                builder_->ScheduleNonsequentialConstruction(addon->unit_type);
            }

            // Schedule the building for reconstruction
            builder_->ScheduleConstructionInRecommendedQueue(unit_->unit_type.ToType(), true);
            return;
    }
}
