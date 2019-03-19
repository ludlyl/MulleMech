
// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Units.h"
#include "core/API.h"

#include <limits>

Units::Units(sc2::Units&& units_) : m_units(std::move(units_)) {
    m_wrappedUnits.reserve(m_units.size());
    for (auto& unit : m_units)
        m_wrappedUnits.emplace_back(gAPI->WrapUnit(unit));
}

Units::Units(Units&& units) : m_units(std::move(units.m_units)), m_wrappedUnits(std::move(units.m_wrappedUnits)) { }

std::optional<Unit> Units::GetClosestUnit(const sc2::Point2D& point_) const {
    float distance = std::numeric_limits<float>::max();

    const sc2::Unit* target = nullptr;
    for (const auto& i : *this) {
        float d = sc2::DistanceSquared2D(i->pos, point_);
        if (d < distance) {
            distance = d;
            target = i;
        }
    }
    
    return target ? std::make_optional(gAPI->WrapUnit(target)) : std::nullopt;
}

std::optional<Unit> Units::GetClosestUnit(sc2::Tag tag_) const {
    std::optional<Unit> unit = gAPI->observer().GetUnit(tag_);
    if (!unit)
        return std::nullopt;

    return GetClosestUnit(unit.value()->pos);
}

std::optional<Unit> Units::GetRandomUnit() const {
    if (empty())
        return std::nullopt;
    int index = sc2::GetRandomInteger(0, static_cast<int>(size()) - 1);
    return std::make_optional(gAPI->WrapUnit(at(static_cast<unsigned>(index))));
}

void Units::push_back(const Unit& unit) {
    m_units.push_back(unit);
    m_wrappedUnits.push_back(unit);
}

void Units::emplace_back(Unit&& unit) {
    m_wrappedUnits.emplace_back(std::move(unit));
    m_units.push_back(m_wrappedUnits.back());
}

Units& Units::operator=(Units&& units) {
    m_units = std::move(units.m_units);
    m_wrappedUnits = std::move(units.m_wrappedUnits);
    return *this;
}

void Units::clear() {
    m_units.clear();
    m_wrappedUnits.clear();
}

Units::T::const_iterator Units::erase(T::const_iterator itr) {
    assert(m_units[static_cast<std::size_t>(std::distance(begin(), itr))] == *itr && "Units::erase: m_units[i] matching m_wrappedUnits[i]");
    m_units.erase(m_units.begin() + std::distance(begin(), itr));
    return m_wrappedUnits.erase(itr);
}
