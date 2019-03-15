// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov
#include <memory>
#include "Blueprint.h"
#include "Building.h"
#include "Mutation.h"
#include "Refinery.h"
#include "SupplyDepot.h"
#include "Barrack.h"
#include "Factory.h"
#include "Starport.h"
#include "TownHall.h"
#include "Unit.h"
#include "core/API.h"
#include "core/Errors.h"
#include "Addon.h"

bp::Blueprint::~Blueprint() {
}

std::shared_ptr<bp::Blueprint> bp::Blueprint::Plot(sc2::ABILITY_ID ability_) {
    switch (ability_) {
        // Specially handled Buildings
        case sc2::ABILITY_ID::BUILD_REFINERY:
            return std::make_shared<Refinery>();
        case sc2::ABILITY_ID::BUILD_COMMANDCENTER:
            return std::make_shared<TownHall>();

        case sc2::ABILITY_ID::BUILD_BARRACKS:
            return std::make_shared<Barrack>();

        case sc2::ABILITY_ID::BUILD_FACTORY:
            return std::make_shared<Factory>();

        case sc2::ABILITY_ID::BUILD_STARPORT:
            return std::make_shared<Starport>();

        case sc2::ABILITY_ID::BUILD_SUPPLYDEPOT:
            return std::make_shared<SupplyDepot>();

        // Add-ons
        case sc2::ABILITY_ID::BUILD_TECHLAB_BARRACKS:
        case sc2::ABILITY_ID::BUILD_REACTOR_BARRACKS:
        case sc2::ABILITY_ID::BUILD_TECHLAB_FACTORY:
        case sc2::ABILITY_ID::BUILD_REACTOR_FACTORY:
        case sc2::ABILITY_ID::BUILD_TECHLAB_STARPORT:
        case sc2::ABILITY_ID::BUILD_REACTOR_STARPORT:
            return std::make_shared<Addon>();

        // Barracks
        case sc2::ABILITY_ID::TRAIN_MARINE:
        case sc2::ABILITY_ID::TRAIN_REAPER:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_BARRACKS);

        case sc2::ABILITY_ID::TRAIN_MARAUDER:
        case sc2::ABILITY_ID::TRAIN_GHOST:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_BARRACKS, sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB);

        // Factory
        case sc2::ABILITY_ID::TRAIN_HELLION:
        case sc2::ABILITY_ID::TRAIN_WIDOWMINE:
        case sc2::ABILITY_ID::TRAIN_HELLBAT:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_FACTORY);

        case sc2::ABILITY_ID::TRAIN_THOR:
        case sc2::ABILITY_ID::TRAIN_SIEGETANK:
        case sc2::ABILITY_ID::TRAIN_CYCLONE:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_FACTORY, sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);

        // Starport
        case sc2::ABILITY_ID::TRAIN_VIKINGFIGHTER:
        case sc2::ABILITY_ID::TRAIN_MEDIVAC:
        case sc2::ABILITY_ID::TRAIN_LIBERATOR:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_STARPORT);

        case sc2::ABILITY_ID::TRAIN_RAVEN:
        case sc2::ABILITY_ID::TRAIN_BANSHEE:
        case sc2::ABILITY_ID::TRAIN_BATTLECRUISER:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_STARPORT, sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB);

        case sc2::ABILITY_ID::TRAIN_SCV:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::INVALID);

        // Command Center Mutations
        case sc2::ABILITY_ID::MORPH_ORBITALCOMMAND:
        case sc2::ABILITY_ID::MORPH_PLANETARYFORTRESS:
            return std::make_shared<Mutation>();

        // Upgrades
        // NOTE (alkurbatov): Yes, this is weird from the first glance
        // but anyway the code required for research is completely the same. :)

        // Engineering Bay
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL3:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL3:
        case sc2::ABILITY_ID::RESEARCH_NEOSTEELFRAME:               // Neon-steel Armor
        case sc2::ABILITY_ID::RESEARCH_HISECAUTOTRACKING:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_ENGINEERINGBAY);

        // Armory
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL3:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL3:
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL3:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_ARMORY);

        // Fusion Core
        case sc2::ABILITY_ID::RESEARCH_BATTLECRUISERWEAPONREFIT:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_FUSIONCORE);

        // Ghost Academy
        case sc2::ABILITY_ID::RESEARCH_PERSONALCLOAKING:
        case sc2::ABILITY_ID::BUILD_NUKE:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_GHOSTACADEMY);

        // Barracks with Tech Lab
        case sc2::ABILITY_ID::RESEARCH_COMBATSHIELD:
        case sc2::ABILITY_ID::RESEARCH_STIMPACK:
        case sc2::ABILITY_ID::RESEARCH_CONCUSSIVESHELLS:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB);

        // Factory with Tech Lab
        case sc2::ABILITY_ID::RESEARCH_INFERNALPREIGNITER:          // sc2::UPGRADE_ID::HIGHCAPACITYBARRELS
        // case sc2::ABILITY_ID::MISSING:                           // Mag-Field Accelerator (upgrade id=144, ability id=769; missing in API)
        case sc2::ABILITY_ID::RESEARCH_DRILLINGCLAWS:
        case sc2::ABILITY_ID::RESEARCH_SMARTSERVOS:
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB);

        // Startport with Tech Lab
        case sc2::ABILITY_ID::RESEARCH_HIGHCAPACITYFUELTANKS:       // Rapid Reignition System, sc2::UPGRADE_ID::MEDIVACINCREASESPEEDBOOST
        case sc2::ABILITY_ID::RESEARCH_RAVENCORVIDREACTOR:
        case sc2::ABILITY_ID::RESEARCH_BANSHEECLOAKINGFIELD:
        case sc2::ABILITY_ID::RESEARCH_BANSHEEHYPERFLIGHTROTORS:    // sc2::UPGRADE_ID::BANSHEESPEED
        case sc2::ABILITY_ID::RESEARCH_ADVANCEDBALLISTICS:          //  sc2::UPGRADE_ID::LIBERATORAGRANGEUPGRADE
            return std::make_shared<Unit>(sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB);

        default:
            return std::make_shared<Building>();
    }
}
