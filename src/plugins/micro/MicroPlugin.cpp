#include "MicroPlugin.h"

#include "DefaultUnit.h"
#include "Marine.h"

#include "core/API.h"

std::unique_ptr<MicroPlugin> MicroPlugin::MakePlugin(Unit* unit) {
    switch (unit->unit_type.ToType()) {
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
        return std::make_unique<Marine>(unit);
    default:
        return std::make_unique<DefaultUnit>(unit);
    }
}

MicroPlugin::MicroPlugin(Unit* unit) :
    m_self(unit), m_target(nullptr), m_moving(false)
{
}

void MicroPlugin::OnCombatFrame(Unit* self, const Units& enemies) {
    m_self = self;
    OnCombatStep(enemies);
}

void MicroPlugin::OnCombatOver(Unit* self) {
    m_self = self;
    OnCombatEnded();
    m_target = nullptr;
    m_moving = false;
}

bool MicroPlugin::CanCast(sc2::ABILITY_ID ability_id) {
    if (!m_self)
        return false;
    for (auto& ability : gAPI->query().GetAbilitiesForUnit(m_self).abilities) {
        if (ability.ability_id.ToType() == ability_id)
            return true;
    }
    return false;
}

void MicroPlugin::Attack(Unit* target) {
    if (m_self && !IsAttacking(target)) {
        gAPI->action().Attack(m_self, target);
        m_target = target;
        m_moving = false;
    }
}

void MicroPlugin::MoveTo(const sc2::Point2D& pos) {
    if (m_self) {
        gAPI->action().MoveTo(m_self, pos);
        m_target = nullptr;
        m_moving = true;
    }
}

bool MicroPlugin::HasBuff(sc2::BUFF_ID buff) {
    if (m_self)
        return false;
    return std::find(m_self->buffs.begin(), m_self->buffs.end(), buff) != m_self->buffs.end();
}

void MicroPlugin::Cast(sc2::ABILITY_ID ability) {
    if (m_self && CanCast(ability))
        gAPI->action().Cast(m_self, ability);
}

void MicroPlugin::Cast(sc2::ABILITY_ID ability, const Unit* target) {
    if (m_self && CanCast(ability))
        gAPI->action().Cast(m_self, ability, target);
}

bool MicroPlugin::IsAttacking(const Unit* target) const {
    return m_target == target;
}

bool MicroPlugin::IsMoving() const {
    return m_moving;
}
