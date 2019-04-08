#include "SCV.h"
#include "core/API.h"

SCV::SCV(Unit* unit)
        : MicroPlugin(unit)
{
}

void SCV::OnStep() {
    if (m_self->mineral_contents == 5) {
       Units commandCenters = gAPI->observer().GetUnits(sc2::Unit::Self);

        auto itr = std::remove_if(commandCenters.begin(), commandCenters.end(), [](const Unit* u) {
            return !(u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER || u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS || u->unit_type.ToType() == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND);
        });
        commandCenters.erase(itr, commandCenters.end());
        Cast(sc2::ABILITY_ID::SMART, commandCenters.GetClosestUnit(m_self->pos));
    }
}
void SCV::OnCombatStep(const Units &enemies) {}