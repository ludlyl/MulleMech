#pragma once

#include "core/Unit.h"
#include "core/Order.h"
#include "core/Construction.h"

struct Expansion;

class Worker : public Unit {
public:
    // NOTE: It is the class/plugin that sets the job responsibility to also unset it
    enum class Job {
        unemployed, // "idle" might be a "prettier" name but that might cause confusion
        gathering_minerals,
        gathering_vespene,
        building,
        scouting,
        fighting, // i.e. the worker is in a combat squad
        repairing
    };

    explicit Worker(const sc2::Unit& unit_);

    // The following functions are only valid if the workers is one of our own

    void BuildRefinery(Order* order_, const Unit* geyser_);

    void Build(Order* order_, const sc2::Point2D& point_);

    // Overload for continuing an already started construction
    void Build(const Unit* building_);

    void Repair(const Unit* target_);

    void GatherVespene(const Unit* target_);

    void SetHomeBase(std::shared_ptr<Expansion> base);

    std::shared_ptr<Expansion> GetHomeBase() const;

    // Make SCV go back to gathering minerals
    void Mine();

    void SetAsUnemployed();

    void SetAsScout();

    void SetAsFighter();

    // Will always return unemployed for enemy workers
    Job GetJob() const { return m_job; }

    // The construction order (or building) that the scv is on its way to start or is currently constructing
    std::unique_ptr<Construction> construction = nullptr;

private:
    Job m_job = Job::unemployed;
    std::shared_ptr<Expansion> m_home_base = nullptr;
};
