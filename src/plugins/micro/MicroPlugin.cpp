#include "MicroPlugin.h"
#include "DefaultUnit.h"
#include "Marine.h"
#include "Thor.h"
#include "Reaper.h"
#include "Battlecruiser.h"
#include "Cyclone.h"
#include "SiegeTank.h"
#include "Widowmine.h"
#include "Medivac.h"
#include "Raven.h"
#include "Hub.h"
#include "core/API.h"


std::unique_ptr<MicroPlugin> MicroPlugin::MakePlugin(Unit* unit) {
    switch (unit->unit_type.ToType()) {
        case sc2::UNIT_TYPEID::TERRAN_MARINE:
            return std::make_unique<Marine>(unit);
        case sc2::UNIT_TYPEID::TERRAN_REAPER:
            return std::make_unique<Reaper>(unit);
        case sc2::UNIT_TYPEID::TERRAN_SIEGETANK:
        case sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED: // cannot spawn as this, but might as well have this case
            return std::make_unique<SiegeTank>(unit);
        case sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER:
            return std::make_unique<Battlecruiser>(unit);
        case sc2::UNIT_TYPEID::TERRAN_THOR:
            return std::make_unique<Thor>(unit);
        case sc2::UNIT_TYPEID::TERRAN_CYCLONE:
            return std::make_unique<Cyclone>(unit);
        case sc2::UNIT_TYPEID::TERRAN_WIDOWMINE:
        case sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED: // cannot spawn as this, but might as well have this case
            return std::make_unique<Widowmine>(unit);
        case sc2::UNIT_TYPEID::TERRAN_MEDIVAC:
            return std::make_unique<Medivac>(unit);
        case sc2::UNIT_TYPEID::TERRAN_RAVEN:
            return std::make_unique<Raven>(unit);
        default:
            return std::make_unique<DefaultUnit>(unit);
    }
}

MicroPlugin::MicroPlugin(Unit* unit) :
        m_self(unit), m_target(nullptr), m_moving(false)
{
}

void MicroPlugin::OnCombatFrame(Unit* self, const Units& enemies,
    const Units& allies, const sc2::Point2D& attackMovePos) {
    m_self = self;
    m_attackMovePos = attackMovePos;

    // Request scan logic
    for (auto& enemy : enemies) {
        if (self->CanAttack(enemy) == Unit::Attackable::need_scan) {
            gHub->RequestScan(enemy->pos);
            break;
        }
    }

    OnCombatStep(enemies, allies);
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

void MicroPlugin::Attack(const Unit* target) {
    if (m_self && !IsAttacking(target)) {
        gAPI->action().Attack(m_self, target);
        m_target = target;
        m_moving = false;
    }
}

void MicroPlugin::AttackMove() {
    AttackMove(m_attackMovePos);
}

void MicroPlugin::AttackMove(const sc2::Point2D& pos) {
    if (m_self && !IsAttackMoving(pos)) {
        if (!CanCast(sc2::ABILITY_ID::ATTACK_ATTACK))
            gAPI->action().Cast(m_self, sc2::ABILITY_ID::ATTACK, pos); // called Scan Attack in-game
        else
            gAPI->action().Attack(m_self, pos);
        m_target = nullptr;
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

void MicroPlugin::Cast(sc2::ABILITY_ID ability, bool queue) {
    if (m_self && CanCast(ability))
        gAPI->action().Cast(m_self, ability, queue);
}

void MicroPlugin::Cast(sc2::ABILITY_ID ability, const Unit* target, bool queue) {
    if (m_self && CanCast(ability))
        gAPI->action().Cast(m_self, ability, target, queue);
}

void MicroPlugin::Cast(sc2::ABILITY_ID ability, const sc2::Point2D& point, bool queue) {
    if (m_self && CanCast(ability))
        gAPI->action().Cast(m_self, ability, point, queue);
}

bool MicroPlugin::IsAttacking(const Unit* target) const {
    return m_target == target;
}

bool MicroPlugin::IsMoving() const {
    return m_moving;
}

bool MicroPlugin::IsAttackMoving() const {
    return m_self && !m_target &&
        !m_self->GetPreviousStepOrders().empty() &&
        m_self->GetPreviousStepOrders().front().ability_id == sc2::ABILITY_ID::ATTACK;
}

bool MicroPlugin::IsAttackMoving(const sc2::Point2D& pos) const {
    return IsAttackMoving() &&
        sc2::DistanceSquared2D(pos, m_self->GetPreviousStepOrders().front().target_pos) <=
        AttackMoveOutOfDateDistance * AttackMoveOutOfDateDistance;
}

