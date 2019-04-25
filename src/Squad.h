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

    Units& GetAllies() { return m_allies; }
    const Units& GetAllies() const { return m_allies; }
    void SetAllies(Units allies) { m_allies = std::move(allies); }


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

    int GetId() const { return m_id; }

    const sc2::Point2D& GetApproachPoint() const { return m_approachPos; }

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
    Units m_allies;
    sc2::Point2D m_center;                          // center of our units
    sc2::Point2D m_regroupPos;
    sc2::Point2D m_approachPos;
    float m_spreadRadius = 0;                       // how spread out our units are from the center
    MovementState m_moveState = MovementState::idle;
    bool m_wasApproaching = false;                  // true if we were approaching before regroup command
    int m_id;

    static constexpr float RegroupRadius = 15.0f;   // while Approach()'ing, regroup if we achieve this much spread
};
