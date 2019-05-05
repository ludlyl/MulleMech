#pragma once

#include "core/Units.h"

#include <memory>
#include <string>

class Squad {
public:
    Squad();
    virtual ~Squad() = default;

    virtual bool IsTaskFinished() const = 0;

    void OnStep();

    void AddUnit(Unit* unit);
    void RemoveUnit(Unit* unit);
    Units& GetUnits() { return m_units; }
    const Units& GetUnits() const { return m_units; }
    void SetUnits(Units units) { m_units = std::move(units); }

    // Move units from other to this squad
    void Absorb(Squad& other);

    void AddEnemy(Unit* enemy);
    void RemoveEnemey(Unit* enemy);
    Units& GetEnemies() { return m_enemies; }
    const Units& GetEnemies() const { return m_enemies; }
    void SetEnemies(Units enemies) { m_enemies = std::move(enemies); }

    // Approach a position as a group; units are kept from getting too spread out
    void Approach(const sc2::Point2D& position);

    void RegroupAt(const sc2::Point2D& position);

    bool IsMoving() const { return m_moveState != MovementState::idle; }

    void AbortMovement();

    int Size() const;

    // Center point of our units
    const sc2::Point2D& GetCenter() const { return m_center; }

    // How spread out our units are
    float GetSpreadRadius() const { return m_spreadRadius; }

    // Center point of enemies (only defined to sensible value if we have enemies)
    const sc2::Point2D& GetEnemyCenter() const { return m_enemyCenter; }

    // How spread out enemies are (only defined to sensible value if we have enemies)
    float GetEnemySpreadRadius() const { return m_enemySpreadRadius; }

    int GetId() const { return m_id; }

    const sc2::Point2D& GetApproachPoint() const { return m_approachPos; }

    // Sensible A-move location for any unit in this squad
    const sc2::Point2D& GetAttackMovePoint() const;

protected:
    virtual void Update() = 0;

    virtual std::string SquadName() const = 0;

private:
    void CalculateCenter();

    void UpdateMovement();

    void IssueMoveCommand(const sc2::Point2D& position);

    enum class MovementState {
        idle,
        approach,
        regroup
    };

    Units m_units;
    Units m_enemies;
    sc2::Point2D m_center;                          // center of our units
    sc2::Point2D m_enemyCenter;                     // center of enemy units
    sc2::Point2D m_regroupPos;
    sc2::Point2D m_approachPos;
    float m_spreadRadius = 0;                       // how spread out our units are from the center
    float m_enemySpreadRadius = 0;                  // how spread enemies are from their center
    MovementState m_moveState = MovementState::idle;
    bool m_wasApproaching = false;                  // true if we were approaching before regroup command
    int m_id;

    static constexpr float RegroupRadius = 15.0f;   // while Approach()'ing, regroup if we achieve this much spread
};
