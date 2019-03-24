//
// Created by kevin on 2019-03-07.
//
#include "plugins/Plugin.h"
#include <unordered_set>
#include <vector>
#include "DefaultUnit.h"

class Reaper : public Plugin {
public:

    Reaper();

    void OnStep(Builder *builder) final;

    void OnUnitCreated(const Unit& unit_) final;

    void OnUnitDestroyed(const Unit& unit_, Builder *) final;


private:
    sc2::Units m_reapers;
};