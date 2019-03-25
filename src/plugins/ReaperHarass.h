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

    void OnUnitIdle(const Unit& unit_, Builder*) final;

    void OnUnitDestroyed(const Unit& unit_, Builder*) final;

    void OnUnitCreated(const Unit& unit_) final;



private:

    sc2::Units m_reaperStrikeTeam;
    bool strikeInProgress;
};
