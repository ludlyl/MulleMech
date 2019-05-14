#include "SCV.h"
#include "core/API.h"

SCV::SCV(Unit* unit) : MicroPlugin(unit) {}

void SCV::OnCombatStep(const Units& enemies, const Units& allies) {
    float allies_mechanical_value = 0;

    for (auto& unit : allies) {
        if (unit->unit_type != sc2::UNIT_TYPEID::TERRAN_SCV && unit->HasAttribute(sc2::Attribute::Mechanical)) {
            allies_mechanical_value += unit->GetValue();
        }
    }

    if (allies_mechanical_value > ActiveAutoRepairUnitValueThreshold) {
        // Active auto repair

    } else {
        // Deactivate auto repair and then queue attackmove
    }
}
