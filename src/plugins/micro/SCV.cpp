#include <core/Helpers.h>
#include "SCV.h"
#include "core/API.h"

SCV::SCV(Unit* unit)
        : MicroPlugin(unit) {}

void SCV::OnCombatStep(const Units& enemies, const Units& allies) {

    Units damaged_mechanical_allies = allies;

    auto itr = std::remove_if(damaged_mechanical_allies.begin(), damaged_mechanical_allies.end(), [](const Unit* u) {
        return (!(IsMechanical()(*u)) || !(u->health_max > u->health));
    });

    damaged_mechanical_allies.erase(itr, damaged_mechanical_allies.end());



}