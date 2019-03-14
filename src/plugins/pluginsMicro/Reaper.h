//
// Created by kevin on 2019-03-07.
//
#include "../Plugin.h"
#include <unordered_set>
#include <vector>

class Reaper: public Plugin {
public:

    Reaper();

    void OnStep(Builder* builder) final;
    void OnUnitCreated(const sc2::Unit* unit_) final;
    void OnUnitDestroyed(const sc2::Unit* unit, Builder*) final;


private:
    sc2::Units m_reapers;