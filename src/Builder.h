// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/Order.h"
#include "blueprints/Blueprint.h"

#include <list>

#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit.h>

struct Builder {
    Builder();

    void OnStep();

    void OnUnitCreated(Unit* unit_);

    void OnUnitIdle(Unit* unit_);

    void OnUnitDestroyed(Unit* unit_);

    // Unit is passed for e.g. add-ons
    // Note: Required structures are not scheduled when using this
    // (they are only scheduled when using ScheduleSequentialConstruction)
    void ScheduleNonsequentialConstruction(sc2::UNIT_TYPEID id_, Unit* assignee_ = nullptr);

    void ScheduleSequentialConstruction(sc2::UNIT_TYPEID id_, bool urgent = false, Unit* assignee_ = nullptr);

    // The urgent parameter is only used if the selected queue is the sequential queue
    void ScheduleConstructionInRecommendedQueue(sc2::UNIT_TYPEID id_, bool urgent = false, Unit* assignee_ = nullptr);

    // Upgrades are scheduled in the nonsequential construction queue
    void ScheduleUpgrade(sc2::UPGRADE_ID id_);

    void ScheduleTraining(sc2::UNIT_TYPEID id_, bool urgent = false, Unit* assignee_ = nullptr);

    void ScheduleTrainingOrders(const std::vector<Order>& orders_, bool urgent = false);

    const std::list<Order>& GetNonsequentialConstructionOrders() const;

    const std::list<Order>& GetSequentialConstructionOrders() const;

    const std::list<Order>& GetTrainingOrders() const;

    // Counts all structures scheduled in the seq queue, the non seq queue and all workers
    // with unstarted construction order for the specified building type
    int CountScheduledStructures(sc2::UNIT_TYPEID id_) const;

    int CountScheduledTrainings(sc2::UNIT_TYPEID id_) const;

    bool HasTechRequirements(const Order* order_) const;

 private:
    // If no blueprint is sent in the function will create one
    // Resources = minerals & gas (i.e. supply is not seen as a resource)
    bool AreNoneResourceRequirementsFulfilled(Order* order_, std::shared_ptr<bp::Blueprint> blueprint = nullptr);

    bool Build(Order* order_);

    // Used to prevent deadlock in sequential construction orders
    void ScheduleRequiredStructures(const Order& order_, bool urgent);

    int32_t m_minerals;
    int32_t m_vespene;

    float m_available_food;

    std::list<Order> m_nonsequential_construction_orders;
    std::list<Order> m_sequential_construction_orders;
    std::list<Order> m_training_orders;

    static constexpr int MinimumUnitMineralCost = 50; // I.e. what the cheapest unit's mineral cost is
};
