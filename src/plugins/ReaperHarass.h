//
// Created by kevin on 2019-03-07.
//
#include "Plugin.h"
#include "core/Units.h"
#include <unordered_set>
#include <vector>

class ReaperHarass : public Plugin {
public:

    ReaperHarass();

    void OnStep(Builder* builder) final;

    void OnUnitIdle(Unit* unit_, Builder*) final;

    void OnUnitDestroyed(Unit* unit_, Builder*) final;

    void OnUnitCreated(Unit* unit_) final;



private:

    Units m_reaperStrikeTeam;
    bool strikeInProgress;
};
