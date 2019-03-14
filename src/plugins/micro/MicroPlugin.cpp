#include "MicroPlugin.h"

#include "DefaultUnit.h"
#include "Marine.h"

#include "core/API.h"

std::shared_ptr<MicroPlugin> MicroPlugin::MakePlugin(const sc2::Unit* unit) {
    switch (unit->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
        return std::make_shared<Marine>(unit);
    default:
        return std::make_shared<DefaultUnit>(unit);
    }
}

MicroPlugin::MicroPlugin(const sc2::Unit* unit) :
    m_self(unit), m_moving(false)
{
}

const sc2::Unit* MicroPlugin::Self() {
    assert(m_self != nullptr);
    return m_self;
}

void MicroPlugin::OnCombatFrame(const sc2::Unit* self, const Units& enemies) {
    m_self = self;
    OnCombatStep(enemies);
    m_self = nullptr;
}

void MicroPlugin::OnCombatOver(const sc2::Unit* self) {
    m_self = self;
    OnCombatEnded();
    m_target = sc2::NullTag;
    m_moving = false;
    m_self = nullptr;
}

bool MicroPlugin::CanCast(sc2::ABILITY_ID ability_id) {
    if (!Self())
        return false;
    for (auto& ability : gAPI->query().GetAbilitiesForUnit(*Self()).abilities) {
        if (ability.ability_id.ToType() == ability_id)
            return true;
    }
    return false;
}

void MicroPlugin::Attack(const sc2::Unit* target) {
    if (Self() && !IsAttacking(target)) {
        gAPI->action().Attack(*Self(), *target);
        m_target = target->tag;
        m_moving = false;
    }
}

void MicroPlugin::MoveTo(const sc2::Point2D& pos) {
    if (Self()) {
        gAPI->action().MoveTo(*Self(), pos);
        m_target = sc2::NullTag;
        m_moving = true;
    }
}

bool MicroPlugin::HasBuff(sc2::BUFF_ID buff) {
    if (!Self())
        return false;
    return std::find(Self()->buffs.begin(), Self()->buffs.end(), buff) != Self()->buffs.end();
}

void MicroPlugin::Cast(sc2::ABILITY_ID ability) {
    if (Self() && CanCast(ability))
        gAPI->action().Cast(*Self(), ability);
}

bool MicroPlugin::IsAttacking(const sc2::Unit* target) const {
    return m_target == target->tag;
}

bool MicroPlugin::IsMoving() const {
    return m_moving;
}
