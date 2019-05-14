#include "Thor.h"
#include "core/API.h"
#include "core/Helpers.h"

Thor::Thor(Unit* unit) : MicroPlugin(unit) {
    // High impact mode is generally better
    Cast(sc2::ABILITY_ID::MORPH_THORHIGHIMPACTMODE);
}

void Thor::OnCombatStep(const Units& enemies, const Units& allies) {
    if (m_self->GetPreviousStepOrders().empty() ||
        (m_self->GetPreviousStepOrders().front().ability_id != sc2::ABILITY_ID::MORPH_THORHIGHIMPACTMODE &&
         m_self->GetPreviousStepOrders().front().ability_id != sc2::ABILITY_ID::MORPH_THOREXPLOSIVEMODE)) {
        Units massive_targets_in_range; // Or rather: Massive targets and immortals
        for (auto& enemy : enemies) {
            if (enemy->HasAttribute(sc2::Attribute::Massive) || enemy->unit_type == sc2::UNIT_TYPEID::PROTOSS_IMMORTAL) {
                if ((enemy->is_flying && sc2::Distance2D(m_self->pos, enemy->pos) <= HighImpactPayloadRange) ||
                    (!enemy->is_flying && sc2::Distance2D(m_self->pos, enemy->pos) <= GroundAttackRange)) {
                    massive_targets_in_range.push_back(enemy);
                }
            }
        }

        // A pretty "naive" approach is used here. Instead of just checking the closest unit we should take
        // into account DPS vs ourself and our allies, take into account the angle (turning takes time)
        // take into account our DPS, how damaged the enemy units are etc.
        // Furthermore we probably want to focus fire in the middle of large groups of light flying units,
        // prioritize e.g. stalkers of zerlings in some scenarios etc.

        if (!massive_targets_in_range.empty()) {
            const auto& closest_target = massive_targets_in_range.GetClosestUnit(m_self->pos);
            if (closest_target->is_flying && m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_THOR) {
                Cast(sc2::ABILITY_ID::MORPH_THORHIGHIMPACTMODE);
            } else {
                // As we've already calculated the distance when adding to "massive_targets_in_range" an optimization
                // would be to re-use that value to get the closest unit (instead of calling GetClosestUnit)
                Attack(massive_targets_in_range.GetClosestUnit(m_self->pos));
            }
        } else {
            // Calling this every step is a bit unnecessary
            // (or might even have a negative effect if we keep switching back and forth to often)
            // As it seems possible to switch mode even if we are currently in the process
            // of switching mode it might be good to call this outside of if if check
            if (!SwitchAntiAirWeaponIfNeeded(enemies)) {
                AttackMove();
            }
        }
    }
}

bool Thor::SwitchAntiAirWeaponIfNeeded(const Units& enemies) {
    // An improvement here would be to have different limits for different kinds of units
    // (e.g. if there are 30 vikings we probably want to use explosive mode)
    // Might also want to consider how close the units are to each other
    // This function should also be made to consider the amount of non light flying units in proximity
    // e.g. if there are quite a few void rays and 10 phoenix it's most likely a bad thing to switch to explosive mode
    // (as we currently would do)

    Units light_flying_units_in_proximity;
    for (auto& enemy : enemies) {
        if (!IsTemporaryUnit()(enemy) && enemy->is_flying &&
            enemy->HasAttribute(sc2::Attribute::Light) && sc2::Distance2D(m_self->pos, enemy->pos) <= LightFlyingProximityRange) {
            light_flying_units_in_proximity.push_back(enemy);
        }
    }

    //Turn on explosive mode if there is a significant amount of light flying units
    if (light_flying_units_in_proximity.size() >= ExplosiveModeActivationThreshold) {
        if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_THORAP) {
            Cast(sc2::ABILITY_ID::MORPH_THOREXPLOSIVEMODE);
            return true;
        }
    } else if (light_flying_units_in_proximity.size() <= ExplosiveModeActivationThreshold) {
        if (m_self->unit_type == sc2::UNIT_TYPEID::TERRAN_THOR) {
            Cast(sc2::ABILITY_ID::MORPH_THORHIGHIMPACTMODE);
            return true;
        }
    }
    return false;
}
