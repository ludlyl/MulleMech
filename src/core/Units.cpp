
// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Units.h"
#include "core/API.h"

#include <limits>

Units::Units(const sc2::Units& units_) {
    m_wrappedUnits.reserve(units_.size());
    for (auto& unit : units_)
        m_wrappedUnits.emplace_back(gAPI->WrapUnit(unit));
}

Unit* Units::GetClosestUnit(const sc2::Point2D& point_) const {
    float distance = std::numeric_limits<float>::max();

    Unit* target = nullptr;
    for (const auto& i : m_wrappedUnits) {
        float d = sc2::DistanceSquared2D(i->pos, point_);
        if (d < distance) {
            distance = d;
            target = i;
        }
    }

    return target;
}

const /*std::vector<sc2::Unit*>*/ sc2::Unit* Units::GetSecondClosestUnit(const sc2::Point2D& point_) const {
    float lowestDistance = std::numeric_limits<float>::max();
    float secondLowestDistance = std::numeric_limits<float>::max();

    const sc2::Unit* targetLowest = nullptr;
    const sc2::Unit* targetSecondLowest = nullptr;

    for (const auto& i : m_units) {
        float d = sc2::DistanceSquared2D(i->pos, point_);
        if (d < lowestDistance) {
            secondLowestDistance = lowestDistance;
            lowestDistance = d;
            targetSecondLowest = targetLowest;
            targetLowest = i;
        }
        if (d < secondLowestDistance) {
            secondLowestDistance = d;
            targetSecondLowest = i;
        }
    }

    //std::vector<sc2::Unit*> closeUnits (targetLowest, targetSecondLowest);

    //closeUnits.insert(closeUnits.begin(), targetLowest);
    //closeUnits.insert(closeUnits.begin(), targetSecondLowest);

    return targetSecondLowest;
}

Unit* Units::GetClosestUnit(sc2::Tag tag_) const {
    Unit* unit = gAPI->observer().GetUnit(tag_);
    if (!unit)
        return nullptr;

    return GetClosestUnit(unit->pos);
}

Unit* Units::GetRandomUnit() const {
    if (empty())
        return nullptr;
    int index = sc2::GetRandomInteger(0, static_cast<int>(size()) - 1);
    return m_wrappedUnits[static_cast<unsigned>(index)];
}

sc2::Units Units::ToAPI() const {
    sc2::Units apiUnits;
    apiUnits.reserve(m_wrappedUnits.size());
    for (auto& unit : m_wrappedUnits)
        apiUnits.push_back(unit);
    return apiUnits;
}
