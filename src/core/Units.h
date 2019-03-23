// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Unit.h"
#include <sc2api/sc2_unit.h>
#include <memory>
#include <optional>

class Units {
public:
    using T = std::vector<Unit>;

    Units() { }
    explicit Units(sc2::Units&& units_);
    Units(Units&& units);

    std::optional<Unit> GetClosestUnit(const sc2::Point2D& point_) const;

    std::optional<Unit> GetClosestUnit(sc2::Tag tag_) const;

    std::optional<Unit> GetRandomUnit() const;

    const sc2::Units& ToAPI() const { return m_units; }

    // Common functions found in std::vector implementations
    // T::iterator begin() { return m_wrappedUnits.begin(); }  -- dont allow direct manipulation of Unit element
    // T::iterator end() { return m_wrappedUnits.end(); } -- dito
    T::const_iterator begin() const { return m_wrappedUnits.begin(); }
    T::const_iterator end() const { return m_wrappedUnits.end(); }
    std::size_t size() const { return m_wrappedUnits.size(); }
    bool empty() const { return m_wrappedUnits.empty(); }
    // Unit& at(std::size_t i) { return m_wrappedUnits.at(i); } -- dito
    const Unit& at(std::size_t i) const { return m_wrappedUnits.at(i); }
    // Unit& operator[](std::size_t i) { return m_wrappedUnits[i]; } -- dito
    const Unit& operator[](std::size_t i) const { return m_wrappedUnits[i]; }
    // Unit& front() { return m_wrappedUnits.front(); } -- dito
    const Unit& front() const { return m_wrappedUnits.front(); }
    // Unit& back() { return m_wrappedUnits.back(); } -- dito
    const Unit& back() const { return m_wrappedUnits.back(); }

    void push_back(const Unit& unit);
    void emplace_back(Unit&& unit);
    Units& operator=(Units&& units);
    void clear();
    T::const_iterator erase(T::const_iterator it);

private:
    sc2::Units m_units;
    T m_wrappedUnits;
};
