//
// Created by kevin on 2019-03-07.
//
#include "plugins/Plugin.h"
#include "plugins/micro/MicroPlugin.h"
#include <unordered_set>
#include <vector>
#include "DefaultUnit.h"

class Reaper : public Plugin {
public:

    Reaper();

    void OnStep(Builder *builder) final;

    void OnUnitCreated(Unit* unit_) final;

    void OnUnitDestroyed(Unit* unit_, Builder *) final;


private:
    Units m_reapers;
};