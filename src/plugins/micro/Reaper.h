#include "plugins/Plugin.h"
#include "plugins/micro/MicroPlugin.h"
#include "core/Units.h"
#include "DefaultUnit.h"

#include <unordered_set>
#include <vector>

class Reaper : public Plugin {
public:

    Reaper();

    void OnStep(Builder *builder) final;

    void OnUnitCreated(Unit* unit_) final;

    void OnUnitDestroyed(Unit* unit_, Builder *) final;


private:
    Units m_reapers;
};