
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

    void AddDefenders(Units& defenders, const sc2::Point2D& location, int needed_antiair_resources,
        int needed_remaining_resources, int our_value, int their_value);

    sc2::Point3D GetArmyIdlePosition() const;
    std::vector<sc2::Point2D> GetListOfAttackPoints();
    void UpdateMainAttackTarget();

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

    // Supposed to approximately match units' vision radius
    // (all workers have a vision radius of 8 and most other units have either the same or higher radius)
    static constexpr int ApproximateUnitVisionRadius = 8;
    static constexpr float SearchEnemyPadding = 25.0f;          // Defend this far from our buildings
    static constexpr int AttackOnSupply = 190;                  // Applicable under PlayStyle::normal
    static constexpr float IdleDistance = 10.0f;                // Idle this far from a Command Center
    static constexpr int HarassOnCount = 4;                     // Send harass squad with this many units
    static constexpr int ReinforceOnCount = 6;                  // Send reinforce squads with this many units
    static constexpr float ReinforceSquadDist = 50.0f;          // Use ReinforceSquad if main squad is this far away
    static constexpr float DefenseResourcesOveredo = 1.25f;     // Spend this much more resources on defense compared to enemy
    static constexpr float DefendWithAllValueRatio = 0.25f;     // If enemy value to our value ratio is this or above => involve entire mainsquad
    // Max length in seconds since we last saw the building for it to be used as an attack point
    // This is needed as some buildings might die while outside of our vision (e.g. burn down)
    // and IntelligenceHolder only clear units that we've seen die
    static constexpr int AttackTargetBuildingLastSeenInThreshold = 120;
};
