#include "DefaultUnit.h"
#include "core/API.h"

DefaultUnit::DefaultUnit(Unit* unit)
    : MicroPlugin(unit) {}

void DefaultUnit::OnCombatStep(const Units& enemies, const Units& allies) {
    AttackMove();
}
