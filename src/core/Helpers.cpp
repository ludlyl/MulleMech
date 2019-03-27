// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Helpers.h"

#include "API.h"
#include "Converter.h"
#include "Hub.h"
#include "API.h"

#include <cmath>

IsUnit::IsUnit(sc2::UNIT_TYPEID type_, bool with_not_finished):
    m_type(type_), m_build_progress(1.0f) {
    if (with_not_finished)
        m_build_progress = 0.0f;
}

bool IsUnit::operator()(const sc2::Unit& unit_) const {
    // Note for future development of function:
    // Returning true if the unit is a tech alias as default is NOT ok here
    // (as that would break e.g. IsIdleUnit (a flying factory can be assigned to produce a helion))
    // Instead add another parameter to the constructor or create a new Helper function
    return unit_.unit_type == m_type &&
        unit_.build_progress >= m_build_progress;
}

bool IsCombatUnit::operator()(const sc2::Unit& unit_) const {
    switch (unit_.unit_type.ToType()) {
       case sc2::UNIT_TYPEID::TERRAN_BANSHEE:
       case sc2::UNIT_TYPEID::TERRAN_CYCLONE:
       case sc2::UNIT_TYPEID::TERRAN_GHOST:
       case sc2::UNIT_TYPEID::TERRAN_HELLION:
       case sc2::UNIT_TYPEID::TERRAN_HELLIONTANK:
       case sc2::UNIT_TYPEID::TERRAN_LIBERATOR:
       case sc2::UNIT_TYPEID::TERRAN_LIBERATORAG:
       case sc2::UNIT_TYPEID::TERRAN_MARAUDER:
       case sc2::UNIT_TYPEID::TERRAN_MARINE:
       case sc2::UNIT_TYPEID::TERRAN_MEDIVAC:
       case sc2::UNIT_TYPEID::TERRAN_RAVEN:
       case sc2::UNIT_TYPEID::TERRAN_REAPER:
       case sc2::UNIT_TYPEID::TERRAN_SIEGETANK:
       case sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
       case sc2::UNIT_TYPEID::TERRAN_THOR:
       case sc2::UNIT_TYPEID::TERRAN_THORAP:
       case sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT:
       case sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
       case sc2::UNIT_TYPEID::TERRAN_WIDOWMINE:
       case sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED:

       case sc2::UNIT_TYPEID::ZERG_BANELING:
       case sc2::UNIT_TYPEID::ZERG_BANELINGBURROWED:
       case sc2::UNIT_TYPEID::ZERG_BROODLORD:
       case sc2::UNIT_TYPEID::ZERG_CORRUPTOR:
       case sc2::UNIT_TYPEID::ZERG_HYDRALISK:
       case sc2::UNIT_TYPEID::ZERG_HYDRALISKBURROWED:
       case sc2::UNIT_TYPEID::ZERG_INFESTOR:
       case sc2::UNIT_TYPEID::ZERG_INFESTORBURROWED:
       case sc2::UNIT_TYPEID::ZERG_INFESTORTERRAN:
       case sc2::UNIT_TYPEID::ZERG_LURKERMP:
       case sc2::UNIT_TYPEID::ZERG_LURKERMPBURROWED:
       case sc2::UNIT_TYPEID::ZERG_MUTALISK:
       case sc2::UNIT_TYPEID::ZERG_RAVAGER:
       case sc2::UNIT_TYPEID::ZERG_ROACH:
       case sc2::UNIT_TYPEID::ZERG_ROACHBURROWED:
       case sc2::UNIT_TYPEID::ZERG_ULTRALISK:
       case sc2::UNIT_TYPEID::ZERG_VIPER:
       case sc2::UNIT_TYPEID::ZERG_ZERGLING:
       case sc2::UNIT_TYPEID::ZERG_ZERGLINGBURROWED:

       case sc2::UNIT_TYPEID::PROTOSS_ADEPT:
       case sc2::UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT:
       case sc2::UNIT_TYPEID::PROTOSS_ARCHON:
       case sc2::UNIT_TYPEID::PROTOSS_CARRIER:
       case sc2::UNIT_TYPEID::PROTOSS_COLOSSUS:
       case sc2::UNIT_TYPEID::PROTOSS_DARKTEMPLAR:
       case sc2::UNIT_TYPEID::PROTOSS_DISRUPTOR:
       case sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED:
       case sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR:
       case sc2::UNIT_TYPEID::PROTOSS_IMMORTAL:
       case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP:
       case sc2::UNIT_TYPEID::PROTOSS_ORACLE:
       case sc2::UNIT_TYPEID::PROTOSS_PHOENIX:
       case sc2::UNIT_TYPEID::PROTOSS_SENTRY:
       case sc2::UNIT_TYPEID::PROTOSS_STALKER:
       case sc2::UNIT_TYPEID::PROTOSS_TEMPEST:
       case sc2::UNIT_TYPEID::PROTOSS_VOIDRAY:
       case sc2::UNIT_TYPEID::PROTOSS_ZEALOT:
            return true;

       default:
            return false;
    }
}

bool IsBuilding::operator()(const sc2::Unit& unit_) const {
    return (*this)(unit_.unit_type);
}

bool IsBuilding::operator()(const sc2::UNIT_TYPEID unitTypeid_) const {
    // NOTE: All units except overlord, larva & eggs require food,
    // thus we can use that to assume what is a building and what's not
    auto data = gAPI->observer().GetUnitTypeData(unitTypeid_);
    return data.food_required == 0 &&
           unitTypeid_ != sc2::UNIT_TYPEID::ZERG_OVERLORD &&
           unitTypeid_ != sc2::UNIT_TYPEID::ZERG_OVERSEER &&
           unitTypeid_ != sc2::UNIT_TYPEID::ZERG_OVERLORDTRANSPORT &&
           unitTypeid_ != sc2::UNIT_TYPEID::ZERG_LARVA &&
           unitTypeid_ != sc2::UNIT_TYPEID::ZERG_EGG;
}

bool IsVisibleMineralPatch::operator()(const sc2::Unit& unit_) const {
    return unit_.mineral_contents > 0;
}

bool IsFoggyResource::operator()(const sc2::Unit& unit_) const {
    switch (unit_.unit_type.ToType()) {
        // Mineral types.
        case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD:

        // Geyser types.
        case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER:
            return unit_.display_type != sc2::Unit::DisplayType::Visible;

        default:
            return false;
    }
}

bool IsVisibleGeyser::operator()(const sc2::Unit& unit_) const {
    return unit_.vespene_contents > 0;
}

bool IsFreeGeyser::operator()(const sc2::Unit& unit_) const {
    return IsVisibleGeyser()(unit_) && !gHub->IsOccupied(gAPI->WrapUnit(&unit_));
}

bool IsRefinery::operator()(const sc2::Unit& unit_) const {
    if (unit_.build_progress != 1.0f)
        return false;

    return unit_.unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
        unit_.unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR ||
        unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY;
}

IsIdleUnit::IsIdleUnit(sc2::UNIT_TYPEID type_, bool count_non_full_reactor_as_idle) :
        m_type(type_), m_count_non_full_reactor_as_idle(count_non_full_reactor_as_idle) {
}

bool IsIdleUnit::operator()(const sc2::Unit& unit_) const {
    if (IsUnit(m_type)(unit_)) {
        if (m_count_non_full_reactor_as_idle && HasAddon(sc2::UNIT_TYPEID::TERRAN_REACTOR)(unit_)) {
            return unit_.orders.size() < 2;
        } else {
            return unit_.orders.empty();
        }
    }
    return false;
}

bool IsWorker::operator()(const sc2::Unit& unit_) const {
    return unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_SCV ||
        unit_.unit_type == sc2::UNIT_TYPEID::ZERG_DRONE ||
        unit_.unit_type == sc2::UNIT_TYPEID::PROTOSS_PROBE;
}

bool IsGasWorker::operator()(const sc2::Unit& unit_) const {
    if (!IsWorker()(unit_))
        return false;

    if (unit_.orders.empty())
        return false;

    if (unit_.orders.front().ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
        if (unit_.buffs.empty())
            return false;

        return unit_.buffs.front() == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGAS ||
            unit_.buffs.front() == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASZERG ||
            unit_.buffs.front() == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASPROTOSS;
    }

    if (unit_.orders.front().ability_id == sc2::ABILITY_ID::HARVEST_GATHER)
        return gHub->IsTargetOccupied(unit_.orders.front());

    return false;
}

bool IsTownHall::operator()(const sc2::Unit& unit_) const {
    return unit_.unit_type == sc2::UNIT_TYPEID::PROTOSS_NEXUS ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS ||
           unit_.unit_type == sc2::UNIT_TYPEID::ZERG_HATCHERY ||
           unit_.unit_type == sc2::UNIT_TYPEID::ZERG_HIVE ||
           unit_.unit_type == sc2::UNIT_TYPEID::ZERG_LAIR;
}

bool IsIdleTownHall::operator()(const sc2::Unit& unit_) const {
    return IsTownHall()(unit_) && unit_.orders.empty() && unit_.build_progress == 1.0f;
}

bool IsCommandCenter::operator()(const sc2::Unit& unit_) const {
    return unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS;
}

IsOrdered::IsOrdered(sc2::UNIT_TYPEID type_): m_type(type_) {
}

bool IsOrdered::operator()(const Order& order_) const {
    return order_.unit_type_id == m_type;
}

bool IsWithinDist::operator()(const sc2::Unit& unit_) const {
    return sc2::DistanceSquared3D(m_center, unit_.pos) < m_distSq;
}

HasAddon::HasAddon(sc2::UNIT_TYPEID addon_type_): m_addon_type(addon_type_) {
}

bool HasAddon::operator()(const sc2::Unit& unit_) const {
    // I.e. does unit_ have "no add-on" (INVALID)?
    if (unit_.add_on_tag == sc2::NullTag && m_addon_type == sc2::UNIT_TYPEID::INVALID) {
        return true;
    }
    if (unit_.add_on_tag == sc2::NullTag && m_addon_type != sc2::UNIT_TYPEID::INVALID) {
        return false;
    }

    auto addonAsUnit = gAPI->observer().GetUnit(unit_.add_on_tag);
    auto addonType = addonAsUnit->unit_type.ToType();
    // The second part (after the or) is needed for the function to return true
    // if you send in e.g. just TECHLAB (instead of e.g. FACTORY_TECHLAB)
    return addonType == m_addon_type || gAPI->observer().GetUnitTypeData(addonType).tech_alias.front() == m_addon_type;
}

MultiFilter::MultiFilter(Selector selector, std::initializer_list<std::function<bool(const sc2::Unit& unit)>> fns_)
    : m_functors(fns_), m_selector(selector)
{
}

bool MultiFilter::operator()(const sc2::Unit& unit_) const {
    if (m_selector == Selector::And) {
        for (auto& fn : m_functors) {
            if (!fn(unit_))
                return false;
        }
        return true;
    }
    else if (m_selector == Selector::Or) {
        for (auto& fn : m_functors) {
            if (fn(unit_))
                return true;
        }
        return false;
    }

    return false;
}

sc2::Point2D GetTerranAddonPosition(const Unit* unit_) {
    sc2::Point2D pos = unit_->pos;
    pos.x += ADDON_DISPLACEMENT_IN_X;
    pos.y += ADDON_DISPLACEMENT_IN_Y;
    return pos;
}

std::vector<sc2::Point2D> PointsInCircle(float radius, const sc2::Point2D& center, int numPoints) {
    std::vector<sc2::Point2D> points;
    points.reserve(static_cast<std::vector<sc2::Point2D>::size_type>(numPoints));

    float angleSplit = F_2PI / numPoints;
    for (int i = 0; i < numPoints; ++i) {
        sc2::Point2D p;

        p.x = std::cos(i * angleSplit) * radius;
        p.y = std::sin(i * angleSplit) * radius;
        p += center;

        points.push_back(p);
    }

    return points;
}

std::vector<sc2::Point2D> PointsInCircle(float radius, const sc2::Point2D& center, float forcedHeight, int numPoints) {
    std::vector<sc2::Point2D> points;
    points.reserve(static_cast<std::vector<sc2::Point2D>::size_type>(numPoints));

    float angleSplit = F_2PI / numPoints;
    for (int i = 0; i < numPoints; ++i) {
        sc2::Point2D p;

        // At most 4 attempts per point
        bool found = false;
        for (int j = 0; j < 4 && !found; ++j) {
            // Reduce radius a bit for each attempt
            float magnitude = radius - j * (radius / 5.0f);
            p.x = std::cos(i * angleSplit) * magnitude;
            p.y = std::sin(i * angleSplit) * magnitude;
            auto testVec = p * (magnitude + 0.75f) / magnitude; // Test with a slightly longer vector
            testVec += center;
            p += center;

            if (std::abs(gAPI->observer().TerrainHeight(testVec) - forcedHeight) < 0.05f)
                found = true;
        }

        if (found)
            points.push_back(p);
    }

    return points;
}

sc2::Point2D Rotate2D(sc2::Point2D vector, float rotation) {
    sc2::Point2D vector_prime;
    float cos_angle = std::cos(rotation);
    float sin_angle = std::sin(rotation);

    vector_prime.x = vector.x * cos_angle - vector.y * sin_angle;
    vector_prime.y = vector.x * sin_angle + vector.y * cos_angle;

    return vector_prime;
}

std::vector<sc2::UnitTypeID> GetAllTechRequirements(sc2::UnitTypeID id_) {
    return GetAllTechRequirements(gAPI->observer().GetUnitTypeData(id_));
}

std::vector<sc2::UnitTypeID> GetAllTechRequirements(const sc2::UnitTypeData &data_) {
    return GetAllTechRequirements(data_.ability_id.ToType(), data_.tech_requirement);
}

std::vector<sc2::UnitTypeID> GetAllTechRequirements(sc2::AbilityID id_, sc2::UnitTypeID suppliedTechRequirement_) {
    switch (id_.ToType()) {
        case sc2::ABILITY_ID::RESEARCH_COMBATSHIELD:
        case sc2::ABILITY_ID::RESEARCH_CONCUSSIVESHELLS:
        case sc2::ABILITY_ID::RESEARCH_STIMPACK:
            return {sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB};

        case sc2::ABILITY_ID::RESEARCH_PERSONALCLOAKING:
            return {sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB, sc2::UNIT_TYPEID::TERRAN_GHOSTACADEMY};

        case sc2::ABILITY_ID::RESEARCH_INFERNALPREIGNITER:
            return {sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB};

        case sc2::ABILITY_ID::RESEARCH_DRILLINGCLAWS:
            return {sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB, sc2::UNIT_TYPEID::TERRAN_ARMORY};

        case sc2::ABILITY_ID::RESEARCH_HIGHCAPACITYFUELTANKS:
        case sc2::ABILITY_ID::RESEARCH_RAVENCORVIDREACTOR:
        case sc2::ABILITY_ID::RESEARCH_BANSHEECLOAKINGFIELD:
        case sc2::ABILITY_ID::RESEARCH_BANSHEEHYPERFLIGHTROTORS:
            return {sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB};

        case sc2::ABILITY_ID::RESEARCH_ADVANCEDBALLISTICS:
        case sc2::ABILITY_ID::RESEARCH_BATTLECRUISERWEAPONREFIT:
            return {sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB, sc2::UNIT_TYPEID::TERRAN_FUSIONCORE};

        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_HISECAUTOTRACKING:
        case sc2::ABILITY_ID::RESEARCH_NEOSTEELFRAME:
            return {sc2::UNIT_TYPEID::TERRAN_ENGINEERINGBAY};

        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL3:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL3:
            return {sc2::UNIT_TYPEID::TERRAN_ENGINEERINGBAY, sc2::UNIT_TYPEID::TERRAN_ARMORY};

        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL3:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL3:
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL1:
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL2:
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL3:
            return {sc2::UNIT_TYPEID::TERRAN_ARMORY};

        case sc2::ABILITY_ID::TRAIN_MARAUDER:
            return {sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB};

        case sc2::ABILITY_ID::TRAIN_GHOST:
            return {sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB, sc2::UNIT_TYPEID::TERRAN_GHOSTACADEMY};

        case sc2::ABILITY_ID::TRAIN_HELLBAT:
            return {sc2::UNIT_TYPEID::TERRAN_FACTORY, sc2::UNIT_TYPEID::TERRAN_ARMORY};

        case sc2::ABILITY_ID::TRAIN_CYCLONE:
        case sc2::ABILITY_ID::TRAIN_SIEGETANK:
            return {sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB};

        case sc2::ABILITY_ID::TRAIN_THOR:
            return {sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB, sc2::UNIT_TYPEID::TERRAN_ARMORY};

        case sc2::ABILITY_ID::TRAIN_BANSHEE:
        case sc2::ABILITY_ID::TRAIN_RAVEN:
            return {sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB};

        case sc2::ABILITY_ID::TRAIN_BATTLECRUISER:
            return {sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB, sc2::UNIT_TYPEID::TERRAN_FUSIONCORE};

        default: {
            if (suppliedTechRequirement_ == sc2::UNIT_TYPEID::INVALID) {
                return {};
            }
            return {suppliedTechRequirement_};
        }
    }
}
