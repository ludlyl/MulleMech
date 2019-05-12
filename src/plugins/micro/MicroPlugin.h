#pragma once

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_typeenums.h>

#include <cstdint>
#include <memory>
#include <vector>

class Unit;
class Units;

class MicroPlugin {
public:
    explicit MicroPlugin(Unit* unit);
    virtual ~MicroPlugin() = default;

    void OnCombatFrame(Unit* self, const Units& enemies,
        const Units& allies, const sc2::Point2D& attackMovePos);

    void OnCombatOver(Unit* self);

    static std::unique_ptr<MicroPlugin> MakePlugin(Unit* unit);

protected:
    // Combat step function for MicroPlugin, only processed during combat
    // enemies: List of all enemies we are currently fighting in our localized area (i.e. not irrelevant enemies far away)
    // allies: other allies that are fighting with us in this localized area
    virtual void OnCombatStep(const Units& enemies, const Units& allies) = 0;

    virtual void OnCombatEnded() { }

    bool CanCast(sc2::ABILITY_ID ability_id);

    void Attack(const Unit* target);

    // Attack move towards enemies
    void AttackMove();

    // Attack move towards specific spot
    void AttackMove(const sc2::Point2D& pos);

    void MoveTo(const sc2::Point2D& pos);

    bool HasBuff(sc2::BUFF_ID buff);

    void Cast(sc2::ABILITY_ID ability, bool queue = false);

    void Cast(sc2::ABILITY_ID ability, const Unit* target, bool queue = false);

    void Cast(sc2::ABILITY_ID ability, const sc2::Point2D& point, bool queue = false);

    bool IsAttacking(const Unit* target) const;

    bool IsMoving() const;

    bool IsAttackMoving() const;

    bool IsAttackMoving(const sc2::Point2D& pos) const;

    Unit* m_self;

private:
    const Unit* m_target;
    sc2::Point2D m_attackMovePos;
    bool m_moving;                  // Move command; does not include Attack Move command

    static constexpr float AttackMoveOutOfDateDistance = 10.0f; // Update attack move position if it moves this much
};
