#include "Battlecruiser.h"

#include "core/API.h"

Battlecruiser::Battlecruiser(Unit* unit)
        : MicroPlugin(unit)
{
}

void Battlecruiser::OnCombatStep(const Units& enemies, const Units& allies) {
    if(m_self->health < retreatHealth){
        Cast(sc2::ABILITY_ID::EFFECT_TACTICALJUMP, sc2::Point2D(gAPI->observer().StartingLocation().x,
                                                                gAPI->observer().StartingLocation().y));
    }

    Units copy = enemies;
    //Find priority targets (Valuable targets, usually big units)
    auto itr = std::remove_if(copy.begin(), copy.end(), [](const Unit *u) {
        return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_THOR ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_THORAP ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_CARRIER ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER ||
                 u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_BROODLORD);
    });
    copy.erase(itr, copy.end());
    if (!copy.empty()) {
        Cast(sc2::ABILITY_ID::EFFECT_YAMATOGUN, copy.GetClosestUnit(m_self->pos));
    } else {
        copy = enemies;
        //Find secondary targets (These aren't big units but are dangerous threats to the battlecruiser)
        itr = std::remove_if(copy.begin(), copy.end(), [](const Unit *u) {
            return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_VOIDRAY ||
                     u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_CORRUPTOR);
        });
        copy.erase(itr, copy.end());
        if (!copy.empty()) {
            Cast(sc2::ABILITY_ID::EFFECT_YAMATOGUN, copy.GetClosestUnit(m_self->pos));
        } else {
            copy = enemies;
            //If there are no dangerous units or valuable targets, see if there are static defenses
            itr = std::remove_if(copy.begin(), copy.end(), [](const Unit *u) {
                return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_MISSILETURRET ||
                         u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_BUNKER ||
                         u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON ||
                         u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY ||
                         u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_SPORECRAWLER ||
                         u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_SPINECRAWLER);
            });
            copy.erase(itr, copy.end());
            if (!copy.empty()) {
                Cast(sc2::ABILITY_ID::EFFECT_YAMATOGUN, copy.GetClosestUnit(m_self->pos));
            } else {
                copy = enemies;
                //Finally, check if there is anything else worth bombing (We never want to waste a shot on a single marine or similar, but there might be some larger mech units or similar that we can shoot without completely overkilling it)
                itr = std::remove_if(copy.begin(), copy.end(), [](const Unit *u) {
                    return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_LIBERATOR ||
                             u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_LIBERATORAG ||
                             u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_COLOSSUS ||
                             u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP ||
                             u->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_ULTRALISK);
                });
                copy.erase(itr, copy.end());
                if (!copy.empty()) {
                    Cast(sc2::ABILITY_ID::EFFECT_YAMATOGUN, copy.GetClosestUnit(m_self->pos));
                }
            }
        }
    }
}

