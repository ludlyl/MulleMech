
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
    float m_attack_limit;
    bool m_changedPlayStyle;

    static constexpr float SearchEnemyRadiusPadding = 10.0f;

    Units LookForEnemiesInBase();
    void DefenseCheck();
    void GiveMainSquadNewTask(); //TODO
    void Harass(int limit);

    void PlayNormal();
    void PlayAllIn();
    void PlayOffensive();
    void PlayDefensive();
    void PlayVeryDefensive();
    void PlayGreedy();
    void PlayScout();

};


