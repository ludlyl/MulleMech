
// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Units.h"
#include "core/API.h"

#include <algorithm>
#include <numeric>
#include <limits>

Units::Units(std::vector<Unit*> units_) {
    m_wrappedUnits = std::move(units_);
}

Unit* Units::GetClosestUnit(const sc2::Point2D& point_) {
    return const_cast<Unit*>(const_cast<const Units*>(this)->GetClosestUnit(point_));
}

const Unit* Units::GetClosestUnit(const sc2::Point2D& point_) const {
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

Unit* Units::GetClosestUnit(sc2::Tag tag_) {
    return const_cast<Unit*>(const_cast<const Units*>(this)->GetClosestUnit(tag_));
}

const Unit* Units::GetClosestUnit(sc2::Tag tag_) const {
    Unit* unit = gAPI->observer().GetUnit(tag_);
    if (!unit)
        return nullptr;

    return GetClosestUnit(unit->pos);
}

Unit* Units::GetRandomUnit() {
    return const_cast<Unit*>(const_cast<const Units*>(this)->GetRandomUnit());
}

const Unit* Units::GetRandomUnit() const {
    if (empty())
        return nullptr;
    int index = sc2::GetRandomInteger(0, static_cast<int>(size()) - 1);
    return m_wrappedUnits[static_cast<unsigned>(index)];
}

std::pair<sc2::Point2D, float> Units::CalculateCircle() const {
    if (m_wrappedUnits.empty())
        return std::make_pair(sc2::Point2D(), 0.0f);

    // Centroid of a finite set of points
    sc2::Point2D center = std::accumulate(m_wrappedUnits.begin(), m_wrappedUnits.end(), sc2::Point2D(0, 0),
        [](const sc2::Point2D& p, const Unit* u) {
            return p + u->pos;
        });
    center /= static_cast<float>(m_wrappedUnits.size());

    // Find unit furthest from center
    auto u = std::max_element(m_wrappedUnits.begin(), m_wrappedUnits.end(),
        [&center](const Unit* a, const Unit* b) {
            return DistanceSquared2D(center, a->pos) < DistanceSquared2D(center, b->pos);
        });

    // Use that to calculate radius of circle
    float radius = Distance2D((*u)->pos, center);

    return std::make_pair(center, radius);
}

sc2::Units Units::ToAPI() const {
    sc2::Units apiUnits;
    apiUnits.reserve(m_wrappedUnits.size());
    for (auto& unit : m_wrappedUnits)
        apiUnits.push_back(unit);
    return apiUnits;
}

void Units::remove(const Unit* unit) {
    auto itr = std::find(m_wrappedUnits.begin(), m_wrappedUnits.end(), unit);
    if (itr != m_wrappedUnits.end())
        m_wrappedUnits.erase(itr);
}
