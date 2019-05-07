
#include "Plugin.h"
#include "DefenseSquad.h"
#include "HarassSquad.h"
#include "OffenseSquad.h"
#include "ReinforceSquad.h"
#include "Reasoner.h"
#include <memory>

class CombatCommander : public Plugin {
public:
    CombatCommander();

    void OnStep(Builder* builder) final;

    void OnUnitCreated(Unit* unit_) final;

private:
    std::vector<Units> GroupEnemiesInBase();
    void DefenseCheck();

    void PlayNormal();
    void PlayAllIn();
    void PlayOffensive();
    void PlayDefensive();
    void PlayVeryDefensive();
    void PlayGreedy();
    void PlayScout();

    // Returns new defense group with more defenders (if needed, same group otherwise) given,
    // defenders: current defenders (can be empty)
    // enemies:   group of enemies to defend against
    Units GenerateDefenseFor(Units defenders, const Units& enemies);

    void AddDefenders(Units& defenders, const sc2::Point2D& location, int needed_antiair_resources, int needed_remaining_resources);

    sc2::Point3D GetArmyIdlePosition() const;
    std::vector<sc2::Point2D> GetListOfMapPoints();
    void UpdateAttackTarget();
    bool PointIsReachable(sc2::Point2D point);

    // Returns true if it's unsafe to just run to the main squad
    bool ShouldReinforce(const Unit* unit) const;

    std::vector<DefenseSquad> m_defenseSquads;
    std::vector<ReinforceSquad> m_reinforceSquads;
    std::shared_ptr<OffenseSquad> m_mainSquad;              // Shared with reinforce squads
    HarassSquad m_harassSquad;
    sc2::Point2D m_mainAttackTarget;
    std::vector<sc2::Point2D> m_attackTargets;
    PlayStyle m_playStyle;
    bool m_changedPlayStyle;

    static constexpr float PointDistance = 10.0f;           // Supposed to approximately match units' vision radius
    static constexpr float SearchEnemyPadding = 25.0f;      // Defend this far from our buildings
    static constexpr int AttackOnSupply = 190;              // Applicable under PlayStyle::normal
    static constexpr float IdleDistance = 10.0f;            // Idle this far from a Command Center
    static constexpr int HarassOnCount = 4;                 // Send harass squad with this many units
    static constexpr int ReinforceOnCount = 6;              // Send reinforce squads with this many units
    static constexpr float ReinforceSquadDist = 50.0f;      // Use ReinforceSquad if main squad is this far away
    static constexpr float DefenseResourcesOveredo = 1.25f; // Spend this much more resources on defense comapred to enemy
};
