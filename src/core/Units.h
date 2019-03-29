// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Unit.h"
#include <sc2api/sc2_unit.h>
#include <functional>
#include <memory>
#include <optional>

class Units {
public:
    using T = std::vector<Unit*>;

    Units() { }
    explicit Units(const sc2::Units& units_);

    Unit* GetClosestUnit(const sc2::Point2D& point_) const;

    Unit* GetClosestUnit(sc2::Tag tag_) const;

    Unit* GetRandomUnit() const;

    // Returns a copy of Units as an API-recognizable vector with sc2::Unit objects
    sc2::Units ToAPI() const;

    // Common functions found in std::vector implementations
    T::iterator begin() { return m_wrappedUnits.begin(); }
    T::iterator end() { return m_wrappedUnits.end(); }
    T::const_iterator begin() const { return m_wrappedUnits.begin(); }
    T::const_iterator end() const { return m_wrappedUnits.end(); }
    std::size_t size() const { return m_wrappedUnits.size(); }
    bool empty() const { return m_wrappedUnits.empty(); }
    Unit* at(std::size_t i) { return m_wrappedUnits.at(i); }
    const Unit* at(std::size_t i) const { return m_wrappedUnits.at(i); }
    Unit* operator[](std::size_t i) { return m_wrappedUnits[i]; }
    const Unit* operator[](std::size_t i) const { return m_wrappedUnits[i]; }
    Unit* front() { return m_wrappedUnits.front(); }
    const Unit* front() const { return m_wrappedUnits.front(); }
    Unit* back() { return m_wrappedUnits.back(); }
    const Unit* back() const { return m_wrappedUnits.back(); }
    void push_back(Unit* unit) { m_wrappedUnits.push_back(unit); }
    void clear() { m_wrappedUnits.clear(); }
    T::iterator erase(T::iterator it) { return m_wrappedUnits.erase(it); }
    T::iterator erase(T::iterator a, T::iterator b) { return m_wrappedUnits.erase(a, b); }

    void remove(const Unit* unit);

private:
    T m_wrappedUnits;
};
