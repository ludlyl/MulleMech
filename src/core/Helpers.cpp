// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Helpers.h"
#include "API.h"
#include "Converter.h"
#include "Hub.h"
#include "API.h"

#include <cmath>

IsUnit::IsUnit(sc2::UNIT_TYPEID type_, bool with_not_finished_):
    m_type(type_), m_build_progress(1.0f) {
    if (with_not_finished_)
        m_build_progress = 0.0f;
}

bool IsUnit::operator()(const Unit* unit_) const {
    // Note for future development of function:
    // Returning true if the unit is a tech alias as default is NOT ok here
    // (as that would break e.g. IsIdleUnit (a flying factory can be assigned to produce a helion))
    // Instead add another parameter to the constructor or create a new Helper function
    return unit_->unit_type == m_type &&
        unit_->build_progress >= m_build_progress;
}

bool IsDamaged::operator()(const Unit* unit_) const {
    // Epsilon shouldn (probably) not be needed here
    return unit_->health < unit_->health_max * unit_->build_progress;
}

bool IsCombatUnit::operator()(const Unit* unit_) const {
    // TODO: Check hallucinations

    switch (unit_->unit_type.ToType()) {
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
       case sc2::UNIT_TYPEID::TERRAN_BUNKER:
       case sc2::UNIT_TYPEID::TERRAN_MISSILETURRET:
       case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:

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
       case sc2::UNIT_TYPEID::ZERG_BROODLING:
       case sc2::UNIT_TYPEID::ZERG_LOCUSTMP:
       case sc2::UNIT_TYPEID::ZERG_LOCUSTMPFLYING:
       case sc2::UNIT_TYPEID::ZERG_SPORECRAWLER:
       case sc2::UNIT_TYPEID::ZERG_SPORECRAWLERUPROOTED:
       case sc2::UNIT_TYPEID::ZERG_SPINECRAWLER:
       case sc2::UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED:

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
       case sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON:
            return true;

       default:
            return false;
    }
}

bool IsTemporaryUnit::operator()(const Unit* unit_) const {
    return (*this)(unit_->unit_type);
}

bool IsTemporaryUnit::operator()(sc2::UNIT_TYPEID type_) const {
    // TODO: Check hallucinations

    switch (type_) {
        case sc2::UNIT_TYPEID::TERRAN_MULE:
        case sc2::UNIT_TYPEID::TERRAN_AUTOTURRET:

        case sc2::UNIT_TYPEID::ZERG_INFESTORTERRAN:
        case sc2::UNIT_TYPEID::ZERG_BROODLING:
        case sc2::UNIT_TYPEID::ZERG_LOCUSTMP:
        case sc2::UNIT_TYPEID::ZERG_LOCUSTMPFLYING:

        case sc2::UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT:
        case sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED:
            return true;

        default:
            return false;
    }
}

bool IsAntiAirUnit::operator()(const Unit* unit_) const {
    auto data = gAPI->observer().GetUnitTypeData(unit_->unit_type);
    for (const auto& weapon : data->weapons) {
        if (weapon.type == sc2::Weapon::TargetType::Air || weapon.type == sc2::Weapon::TargetType::Any) {
            return true;
        }
    }
    // Special cases. TODO: Put in more units here (raven? ht?)
    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_WIDOWMINE:
            return true;
        default:
            break;
    }
    return false;
}

bool IsBuilding::operator()(const Unit* unit_) const {
    return (*this)(unit_->unit_type);
}

bool IsBuilding::operator()(sc2::UNIT_TYPEID type_) const {
    auto data = gAPI->observer().GetUnitTypeData(type_);
    // Filter out some units here. Might want to make this into a switch
    // (Would it be better to check IsTemporay?)
    if (type_ == sc2::UNIT_TYPEID::TERRAN_AUTOTURRET)
        return false;
    return std::find(data->attributes.begin(), data->attributes.end(), sc2::Attribute::Structure) != data->attributes.end();
}

bool IsFinishedBuilding::operator()(const Unit* unit_) const {
    return IsBuilding()(unit_) && unit_->build_progress >= 1.0f;
}

bool IsUnfinishedBuilding::operator()(const Unit* unit_) const {
    return IsBuilding()(unit_) && unit_->build_progress < 1.0f;
}

bool IsBuildingWithSupportForAddon::operator()(const Unit* unit_) const {
    return (*this)(unit_->unit_type);
}

bool IsBuildingWithSupportForAddon::operator()(sc2::UNIT_TYPEID type_) const {
    switch (type_) {
        case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSFLYING:
        case sc2::UNIT_TYPEID::TERRAN_FACTORY:
        case sc2::UNIT_TYPEID::TERRAN_FACTORYFLYING:
        case sc2::UNIT_TYPEID::TERRAN_STARPORT:
        case sc2::UNIT_TYPEID::TERRAN_STARPORTFLYING:
            return true;

        default:
            return false;
    }
}

bool IsAddon::operator()(const Unit* unit_) const {
    return (*this)(unit_->unit_type);
}

bool IsAddon::operator()(sc2::UNIT_TYPEID type_) const {
    switch (type_) {
        case sc2::UNIT_TYPEID::TERRAN_TECHLAB:
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSTECHLAB:
        case sc2::UNIT_TYPEID::TERRAN_FACTORYTECHLAB:
        case sc2::UNIT_TYPEID::TERRAN_STARPORTTECHLAB:
        case sc2::UNIT_TYPEID::TERRAN_REACTOR:
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSREACTOR:
        case sc2::UNIT_TYPEID::TERRAN_FACTORYREACTOR:
        case sc2::UNIT_TYPEID::TERRAN_STARPORTREACTOR:
            return true;

        default:
            return false;
    }
}

bool IsVisibleMineralPatch::operator()(const Unit* unit_) const {
    return unit_->mineral_contents > 0;
}

bool IsMineralPatch::operator()(const Unit* unit_) const {
    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750:
        case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD:
        case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750:
            return true;

        default:
            return false;
    }
}

bool IsGeyser::operator()(const Unit* unit_) const {
    switch (unit_->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER:
        case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER:
            return true;
        default:
            return false;
    }
}

bool IsVisibleUndepletedGeyser::operator()(const Unit* unit_) const {
    return unit_->vespene_contents > 0 && unit_->alliance == sc2::Unit::Alliance::Neutral;
}

bool IsFoggyResource::operator()(const Unit* unit_) const {
    if (IsMineralPatch()(unit_) || IsGeyser()(unit_)) {
        return unit_->display_type != sc2::Unit::DisplayType::Visible;
    }
    return false;
}


bool IsRefinery::operator()(const Unit* unit_) const {
    return unit_->unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
           unit_->unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR ||
           unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY;
}

IsIdleUnit::IsIdleUnit(sc2::UNIT_TYPEID type_, bool count_non_full_reactor_as_idle) :
        m_type(type_), m_count_non_full_reactor_as_idle(count_non_full_reactor_as_idle) {
}

bool IsIdleUnit::operator()(const Unit* unit_) const {
    if (IsUnit(m_type)(unit_)) {
        if (m_count_non_full_reactor_as_idle && HasAddon(sc2::UNIT_TYPEID::TERRAN_REACTOR)(unit_)) {
            return unit_->NumberOfOrders() < 2;
        } else {
            return unit_->IsIdle();
        }
    }
    return false;
}

bool IsWorker::operator()(const Unit* unit_) const {
    return unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_SCV ||
           unit_->unit_type == sc2::UNIT_TYPEID::ZERG_DRONE ||
           unit_->unit_type == sc2::UNIT_TYPEID::PROTOSS_PROBE;
}

IsWorkerWithJob::IsWorkerWithJob(Worker::Job job_) : m_job(job_) {
}

bool IsWorkerWithJob::operator()(const Unit* unit_) const {
    if (IsWorker()(unit_)) {
        if (unit_->AsWorker()->GetJob() == m_job) {
            return true;
        }
    }
    return false;
}

IsWorkerWithHomeBase::IsWorkerWithHomeBase(const std::shared_ptr<Expansion>& home_base_) : m_home_base(home_base_) {
}

bool IsWorkerWithHomeBase::operator()(const Unit* unit_) const {
    if (IsWorker()(unit_)) {
        if (unit_->AsWorker()->GetHomeBase() == m_home_base) {
            return true;
        }
    }
    return false;
}

IsWorkerWithUnstartedConstructionOrderFor::IsWorkerWithUnstartedConstructionOrderFor(sc2::UNIT_TYPEID type_) : m_type(type_) {
}

bool IsWorkerWithUnstartedConstructionOrderFor::operator()(const Unit* unit_) const {
    if (IsWorker()(unit_)) {
        auto worker = unit_->AsWorker();
        if (worker->construction && worker->construction->building_type == m_type &&
            worker->construction->building == nullptr) {
            return true;
        }
    }
    return false;
}

bool IsTownHall::operator()(const Unit* unit_) const {
    return unit_->unit_type == sc2::UNIT_TYPEID::PROTOSS_NEXUS ||
           unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
           unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING ||
           unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
           unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING ||
           unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS ||
           unit_->unit_type == sc2::UNIT_TYPEID::ZERG_HATCHERY ||
           unit_->unit_type == sc2::UNIT_TYPEID::ZERG_HIVE ||
           unit_->unit_type == sc2::UNIT_TYPEID::ZERG_LAIR;
}

bool IsIdleTownHall::operator()(const Unit* unit_) const {
    return IsTownHall()(unit_) && unit_->NumberOfOrders() == 0 && unit_->build_progress == 1.0f;
}

IsOrdered::IsOrdered(sc2::UNIT_TYPEID type_): m_type(type_) {
}

bool IsOrdered::operator()(const Order& order_) const {
    return order_.unit_type_id == m_type;
}

bool IsWithinDist::operator()(const Unit* unit_) const {
    if (m_2d)
        return sc2::DistanceSquared2D(m_center, unit_->pos) < m_distSq;
    return sc2::DistanceSquared3D(m_center, unit_->pos) < m_distSq;
}

HasAddon::HasAddon(sc2::UNIT_TYPEID addon_type_): m_addon_type(addon_type_) {
}

bool HasAddon::operator()(const Unit* unit_) const {
    // I.e. does unit_ have "no add-on" (INVALID)?
    if (unit_->add_on_tag == sc2::NullTag && m_addon_type == sc2::UNIT_TYPEID::INVALID) {
        return true;
    }
    if (unit_->add_on_tag == sc2::NullTag && m_addon_type != sc2::UNIT_TYPEID::INVALID) {
        return false;
    }

    auto addonAsUnit = gAPI->observer().GetUnit(unit_->add_on_tag);
    auto addonType = addonAsUnit->unit_type.ToType();
    // The second part (after the or) is needed for the function to return true
    // if you send in e.g. just TECHLAB (instead of e.g. FACTORY_TECHLAB)
    return addonType == m_addon_type || addonAsUnit->GetTypeData()->tech_alias.front() == m_addon_type;
}

MultiFilter::MultiFilter(Selector selector_, std::initializer_list<API::Filter> filters_)
    : m_filters(filters_), m_selector(selector_)
{
}

bool MultiFilter::operator()(const Unit* unit_) const {
    if (m_selector == Selector::And) {
        for (auto& filter : m_filters) {
            if (!filter(unit_))
                return false;
        }
        return true;
    }
    else if (m_selector == Selector::Or) {
        for (auto& filter : m_filters) {
            if (filter(unit_))
                return true;
        }
        return false;
    }
    return false;
}

Inverse::Inverse(API::Filter filter_) : m_filter(std::move(filter_)) {}

bool Inverse::operator()(const Unit* unit_) const {
    return !m_filter(unit_);
}

sc2::Point2D GetTerranAddonPosition(const Unit* unit_) {
    return GetTerranAddonPosition(unit_->pos);
}

sc2::Point2D GetTerranAddonPosition(const sc2::Point2D& parent_building_position_) {
    sc2::Point2D pos = parent_building_position_;
    pos.x += ADDON_DISPLACEMENT_IN_X;
    pos.y += ADDON_DISPLACEMENT_IN_Y;
    return pos;
}

bool CloakState::operator()(const Unit* unit_) const {
    return unit_->cloak == m_state;
}

bool IsThereTooManyEnemiesToBuildAt(const sc2::Point2D& pos_) {
    constexpr float ConsideredDistance = 20.0f;
    constexpr std::size_t ErrorOnEnemyCount = 3;

    return gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::And,
        {IsCombatUnit(), IsWithinDist(pos_, ConsideredDistance)}), sc2::Unit::Alliance::Enemy).size() >= ErrorOnEnemyCount;
}

std::vector<sc2::Point2D> PointsInCircle(float radius_, const sc2::Point2D& center_, int num_points_) {
    std::vector<sc2::Point2D> points;
    points.reserve(static_cast<std::vector<sc2::Point2D>::size_type>(num_points_));

    float angleSplit = F_2PI / num_points_;
    for (int i = 0; i < num_points_; ++i) {
        sc2::Point2D p;

        p.x = std::cos(i * angleSplit) * radius_;
        p.y = std::sin(i * angleSplit) * radius_;
        p += center_;

        points.push_back(p);
    }

    return points;
}

std::vector<sc2::Point2D> PointsInCircle(float radius_, const sc2::Point2D& center_, float forced_height_, int num_points_) {
    std::vector<sc2::Point2D> points;
    points.reserve(static_cast<std::vector<sc2::Point2D>::size_type>(num_points_));

    float angleSplit = F_2PI / num_points_;
    for (int i = 0; i < num_points_; ++i) {
        sc2::Point2D p;

        // At most 4 attempts per point
        bool found = false;
        for (int j = 0; j < 4 && !found; ++j) {
            // Reduce radius a bit for each attempt
            float magnitude = radius_ - j * (radius_ / 5.0f);
            p.x = std::cos(i * angleSplit) * magnitude;
            p.y = std::sin(i * angleSplit) * magnitude;
            auto testVec = p * (magnitude + 0.75f) / magnitude; // Test with a slightly longer vector
            testVec += center_;
            p += center_;

            if (std::abs(gAPI->observer().TerrainHeight(testVec) - forced_height_) < 0.05f)
                found = true;
        }

        if (found)
            points.push_back(p);
    }

    return points;
}

sc2::Point2D Rotate2D(sc2::Point2D vector_, float rotation_) {
    sc2::Point2D vector_prime;
    float cos_angle = std::cos(rotation_);
    float sin_angle = std::sin(rotation_);

    vector_prime.x = vector_.x * cos_angle - vector_.y * sin_angle;
    vector_prime.y = vector_.x * sin_angle + vector_.y * cos_angle;

    return vector_prime;
}

std::vector<sc2::UnitTypeID> GetAllStructureTechRequirements(sc2::UnitTypeID id_) {
    return GetAllStructureTechRequirements(*gAPI->observer().GetUnitTypeData(id_));
}

std::vector<sc2::UnitTypeID> GetAllStructureTechRequirements(const sc2::UnitTypeData& data_) {
    return GetAllStructureTechRequirements(data_.ability_id.ToType(), data_.tech_requirement);
}

std::vector<sc2::UnitTypeID> GetAllStructureTechRequirements(sc2::AbilityID id_,
                                                             sc2::UnitTypeID supplied_tech_requirements_) {
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
            if (supplied_tech_requirements_ == sc2::UNIT_TYPEID::INVALID) {
                return {};
            }
            return {supplied_tech_requirements_};
        }
    }
}

sc2::UPGRADE_ID GetUpgradeTechRequirement(sc2::AbilityID id_) {
    switch (id_.ToType()) {
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL2:
            return sc2::UPGRADE_ID::TERRANINFANTRYARMORSLEVEL1;
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL3:
            return sc2::UPGRADE_ID::TERRANINFANTRYARMORSLEVEL2;
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL2:
            return sc2::UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL1;
        case sc2::ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL3:
            return sc2::UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL2;

        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL2:
            return sc2::UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL1;
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL3:
            return sc2::UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL2;
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL2:
            return sc2::UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL1;
        case sc2::ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL3:
            return sc2::UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL2;
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL2:
            return sc2::UPGRADE_ID::TERRANSHIPWEAPONSLEVEL1;
        case sc2::ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL3:
            return sc2::UPGRADE_ID::TERRANSHIPWEAPONSLEVEL2;

        default:
            return sc2::UPGRADE_ID::INVALID;
    }
}

Units GetFreeWorkers(bool include_gas_workers_) {
    // Might be a bit too ineffective to do it this way
    if (include_gas_workers_) {
        return gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::Or,
                {IsWorkerWithJob(Worker::Job::unemployed),
                 IsWorkerWithJob(Worker::Job::gathering_minerals),
                 IsWorkerWithJob(Worker::Job::gathering_vespene)}), sc2::Unit::Alliance::Self);
    } else {
        return gAPI->observer().GetUnits(MultiFilter(MultiFilter::Selector::Or,
                {IsWorkerWithJob(Worker::Job::unemployed),
                 IsWorkerWithJob(Worker::Job::gathering_minerals)}), sc2::Unit::Alliance::Self);
    }
}

Worker* GetClosestFreeWorker(const sc2::Point2D& location_, bool include_gas_workers_) {
    Unit* closest_unit = GetFreeWorkers(include_gas_workers_).GetClosestUnit(location_);
    if (!closest_unit)
        return nullptr;

    return closest_unit->AsWorker();
}

bool FreeWorkerExists(bool include_gas_workers_) {
    return !GetFreeWorkers(include_gas_workers_).empty();
}
