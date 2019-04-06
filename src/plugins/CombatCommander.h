
#include "Plugin.h"
#include "OffenseSquad.h"
#include "DefenseSquad.h"
#include "Reasoner.h"

class CombatCommander : public Plugin {
public:
    CombatCommander();

    void OnStep(Builder* builder) final;

    void OnUnitCreated(Unit* unit_) final;

    void OnUnitDestroyed(Unit* unit_, Builder*);

private:

    std::vector<DefenseSquad> m_defenseSquads;
    OffenseSquad m_mainSquad;
    OffenseSquad m_harassSquad;
    sc2::Point2D m_mainAttackTarget;
    PlayStyle m_playStyle;
    bool m_changedPlayStyle;

    static constexpr float SearchEnemyRadiusPadding = 10.0f;
    static constexpr float EnemyIsolationDistance = 20.0f;  // Enemies this far apart => different groups
    static constexpr int AttackOnSupply = 190;              // Applicable under PlayStyle::normal

    std::vector<Units> IsolateEnemiesInBase();
    void DefenseCheck();
    void Harass(int limit);

    void PlayNormal();
    void PlayAllIn();
    void PlayOffensive();
    void PlayDefensive();
    void PlayVeryDefensive();
    void PlayGreedy();
    void PlayScout();

    // TODO: Should be replaced by intelligent selecting of what units go to defend
    bool StealUnitFromMainSquad(Units& defenders);
};

