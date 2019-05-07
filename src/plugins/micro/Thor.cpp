#include "Thor.h"

#include "core/API.h"

Thor::Thor(Unit* unit)
        : MicroPlugin(unit)
{
}

void Thor::OnCombatStep(const Units& enemies, const Units& allies) {
    Units copy = enemies;
    //Remove cannon fodder from potential targets
    auto itr = std::remove_if(copy.begin(), copy.end(), [](const Unit *u) {
        return (u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_MARINE ||
                u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_REAPER ||
                u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_SCV ||
                u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_ZEALOT ||
                u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_SENTRY ||
                u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_PROBE ||
                u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_ZERGLING ||
                u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_DRONE);
    });
    copy.erase(itr, copy.end());

    if (!m_self->GetPreviousStepOrders().empty() &&
        m_self->GetPreviousStepOrders().front().ability_id != sc2::ABILITY_ID::MORPH_THORHIGHIMPACTMODE &&
        m_self->GetPreviousStepOrders().front().ability_id != sc2::ABILITY_ID::MORPH_THOREXPLOSIVEMODE) {
        const Unit *target = nullptr;
        if (!copy.empty()) {
            target = copy.GetClosestUnit(m_self->pos);
        }
            //If there is no target besides cannon fodder, shoot any nearby unit
        else if (!enemies.empty()) {
            target = enemies.GetClosestUnit(m_self->pos);
        }

        if (target) {
            Attack(target);
        }
    }

    //Switch modes if there is a significant presence of Mutalisks, Banshees, ravens, pheonixes or Oracles
    auto itr2 = std::remove_if(copy.begin(), copy.end(), [](const Unit *u) {
        return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_BANSHEE ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_RAVEN ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_ORACLE ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_PHOENIX ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_MUTALISK);
    });
    copy.erase(itr2, copy.end());

    //Switch back once there are no more squishy units
    if (copy.empty()) {
        if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_THOR) {
            Cast(sc2::ABILITY_ID::MORPH_THORHIGHIMPACTMODE);

        }
            //Turn on explosive mode if there is a significant amount of squishy flying units
        else if (copy.size() > MinimumFlyingUnits) {
            if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_THORAP) {
                Cast(sc2::ABILITY_ID::MORPH_THOREXPLOSIVEMODE);
            }
        }
    }
}
