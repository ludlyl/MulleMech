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

    // Unit is passed for e.g. add-ons
    // Note: Required structures are not scheduled when using this
    // (they are only scheduled when using ScheduleSequentialConstruction)
    void ScheduleNonsequentialConstruction(sc2::UNIT_TYPEID id_, Unit *unit_ = nullptr);

    void ScheduleSequentialConstruction(sc2::UNIT_TYPEID id_, bool urgent = false, Unit *unit_ = nullptr);

    // The urgent parameter is only used if the selected queue is the sequential queue
    void ScheduleConstructionInRecommendedQueue(sc2::UNIT_TYPEID id_, bool urgent = false, Unit *unit_ = nullptr);

    // Upgrades are scheduled in the nonsequential construction queue
    void ScheduleUpgrade(sc2::UPGRADE_ID id_);

    void ScheduleTraining(sc2::UNIT_TYPEID id_, bool urgent = false, Unit* unit_ = nullptr);

    void ScheduleTrainingOrders(const std::vector<Order>& orders_, bool urgent = false);

    const std::list<Order>& GetNonsequentialConstructionOrders() const;

    const std::list<Order>& GetSequentialConstructionOrders() const;

    const std::list<Order>& GetTrainingOrders() const;

    int64_t CountScheduledStructures(sc2::UNIT_TYPEID id_) const;

    int64_t CountScheduledTrainings(sc2::UNIT_TYPEID id_) const;

    bool HasTechRequirements(const Order* order_) const;

 private:
    // If no blueprint is sent in the function will create one
    bool AreNoneResourceRequirementsFulfilled(Order* order_, std::shared_ptr<bp::Blueprint> blueprint = nullptr);

    bool Build(Order* order_);

    // Used to prevent deadlock in sequential construction orders
    void ScheduleRequiredStructures(const Order& order_, bool urgent);

    void ResolveMissingWorkers();

    int32_t m_minerals;
    int32_t m_vespene;

    float m_available_food;

    std::list<Order> m_nonsequential_construction_orders;
    std::list<Order> m_sequential_construction_orders;
    std::list<Order> m_training_orders;
};
