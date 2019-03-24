
#pragma once

#include "core/Units.h"
#include <cstdint>
#include <memory>
#include <vector>

class MicroPlugin {
public:
    explicit MicroPlugin(const Unit& unit);
    virtual ~MicroPlugin() = default;

    void OnCombatFrame(Unit self, const Units& enemies);

    void OnCombatOver(Unit self);

    static std::unique_ptr<MicroPlugin> MakePlugin(const Unit& unit);

protected:
    // Combat step function for MicroPlugin, only processed during combat
    // enemies: List of all enemies we are currently fighting in our localized area (i.e. not irrelevant enemies far away)
    // TODO: Should probably have a list of all allies that are part of the fight
    virtual void OnCombatStep(const Units& enemies) = 0;

    virtual void OnCombatEnded() { }

    bool CanCast(sc2::ABILITY_ID ability_id);

    void Attack(const Unit& target);

    void MoveTo(const sc2::Point2D& pos);

    bool HasBuff(sc2::BUFF_ID buff);

    void Cast(sc2::ABILITY_ID ability);

    void Cast(sc2::ABILITY_ID ability, const Unit& target);

    bool IsAttacking(const Unit& target) const;

    bool IsMoving() const;

    Unit m_self;

private:
    sc2::Tag m_target;
    bool m_moving;
};
