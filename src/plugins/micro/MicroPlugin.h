
#pragma once

#include "core/Units.h"
#include <cstdint>
#include <memory>
#include <vector>

class MicroPlugin {
public:
    MicroPlugin(const sc2::Unit* unit);
    virtual ~MicroPlugin() { }

    void OnCombatFrame(const sc2::Unit* self, const Units& enemies);

    void OnCombatOver(const sc2::Unit* self);

    static std::shared_ptr<MicroPlugin> MakePlugin(const sc2::Unit* unit);

protected:
    // Get pointer to our unit. Guaranteed valid during events: OnCombatStep, OnCombatEnded
    const sc2::Unit* Self();

    // Combat step function for MicroPlugin, only processed during combat
    // enemies: List of all enemies we are currently fighting in our localized area (i.e. not irrelevant enemies far away)
    // TODO: Should probably have a list of all allies that are part of the fight
    virtual void OnCombatStep(const Units& enemies) = 0;

    virtual void OnCombatEnded() { }

    bool CanCast(sc2::ABILITY_ID ability_id);

    void Attack(const sc2::Unit* target);

    void MoveTo(const sc2::Point2D& pos);

    bool HasBuff(sc2::BUFF_ID buff);

    void Cast(sc2::ABILITY_ID ability);

    bool IsAttacking(const sc2::Unit* target) const;

    bool IsMoving() const;

private:
    const sc2::Unit* m_self;
    sc2::Tag m_target;
    bool m_moving;
};
