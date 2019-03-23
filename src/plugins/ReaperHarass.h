//
// Created by kevin on 2019-03-07.
//
#include "Plugin.h"
#include <unordered_set>
#include <vector>

class ReaperHarass : public Plugin {
public:

    ReaperHarass();

    void OnStep(Builder* builder) final;

    void OnUnitIdle(const sc2::Unit* unit, Builder*) final;

    void OnUnitDestroyed(const sc2::Unit* unit, Builder*) final;

    void OnUnitCreated(const sc2::Unit* unit_) final;



private:

    sc2::Units m_reaperStrikeTeam;
    bool strikeInProgress;
};
