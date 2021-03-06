#include "Squad.h"
#include "Historican.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Map.h"

Squad::Squad() {
    static int globalId = 0;
    m_id = globalId++;
}

void Squad::OnStep() {
    // Remove dead & lost vision of squad units (NOTE: units that despawn instantly, such as hellion, do not get marked dead)
    auto itr = std::remove_if(m_units.begin(), m_units.end(),
        [](auto* u) {
        return !u->is_alive || !u->IsInVision;
    });
    m_units.erase(itr, m_units.end());

    // Remove dead enemies or enemies that went back into fog of war
    auto jitr = std::remove_if(m_enemies.begin(), m_enemies.end(),
        [](const Unit* u) {
            return !u->is_alive || !u->IsInVision;
        });
    m_enemies.erase(jitr, m_enemies.end());

    CalculateCenter();

    Update();

    UpdateMovement();
}

void Squad::AddUnit(Unit* unit) {
    assert(!m_units.contains(unit) && "Tried adding unit to squad twice");
    gHistory.debug(LogChannel::combat) << SquadName() << " got new friendly unit: " <<
        UnitTypeToName(unit->unit_type) << std::endl;
    m_units.push_back(unit);
}

void Squad::RemoveUnit(Unit* unit) {
    m_units.remove(unit);
}

void Squad::Absorb(Squad& other) {
    for (auto& unit : other.GetUnits()) {
        if (IsWorker()(unit)) { // don't absorb workers, mark them unemployed instead
            unit->AsWorker()->SetAsUnemployed();
        } else {
            gAPI->action().MoveTo(unit, GetCenter()); // send to us
            AddUnit(unit);
        }
    }

    other.GetUnits().clear();
}

void Squad::AddEnemy(Unit* unit) {
    assert(!m_units.contains(unit) && "Tried adding enemy to squad targets twice");
    gHistory.debug(LogChannel::combat) << SquadName() << " got new enemy: " <<
        UnitTypeToName(unit->unit_type) << std::endl;
    m_enemies.push_back(unit);
}

void Squad::RemoveEnemey(Unit* unit) {
    m_enemies.remove(unit);
}

void Squad::RegroupAt(const sc2::Point2D& position) {
    gHistory.debug(LogChannel::combat) << SquadName() << " regrouping" << std::endl;
    m_regroupPos = position;
    m_wasApproaching = m_moveState == MovementState::approach;
    m_moveState = MovementState::regroup;
}

void Squad::Approach(const sc2::Point2D& position) {
    gHistory.debug(LogChannel::combat) << SquadName() << " approaching" << std::endl;
    m_approachPos = position;
    m_moveState = MovementState::approach;
}

void Squad::AbortMovement() {
    if (m_moveState != MovementState::idle)
        gHistory.debug(LogChannel::combat) << SquadName() << " aborted movement" << std::endl;
    m_moveState = MovementState::idle;
    m_wasApproaching = false;
}

void Squad::CalculateCenter() {
    auto pair = m_units.CalculateCircle();
    m_center = pair.first;
    m_spreadRadius = pair.second;

    if (!m_enemies.empty()) {
        auto circle = m_enemies.CalculateCircle();
        m_enemyCenter = circle.first;
        m_enemySpreadRadius = circle.second;
    }
}

void Squad::UpdateMovement() {
    switch (m_moveState) {
        case MovementState::idle:
            break;

        case MovementState::approach:
            if (Distance2D(GetCenter(), m_approachPos) < RegroupRadius / 2.0f) { // TODO: Use a different constant for this?
                m_moveState = MovementState::idle;
                gHistory.debug(LogChannel::combat) << SquadName() << " approach finished, idling" << std::endl;
            } else if (GetSpreadRadius() > RegroupRadius) {
                // If we get too spread out -> regroup
                // We only regroup if the the center position of the squad is reachable
                // Might be worth regrouping at e.g. the middle most ground unit if this fails or something like that
                Unit* regroup_unit = nullptr;
                for (auto& unit : m_units) {
                    if (!unit->is_flying) {
                        regroup_unit = unit;
                    }
                }
                if (!regroup_unit && !m_units.empty()) {
                    // As more units can be added to the squad while it tries to regroup doing,
                    // using a flying unit to check if the regroup position is reachable isn't really safe
                    regroup_unit = m_units.front();
                }
                if (regroup_unit && IsPointReachable(regroup_unit, GetCenter())) {
                    RegroupAt(GetCenter());
                }
            } else {
                // TODO: Slightly unnecessary to reissue the movement command each step
                IssueMoveCommand(m_approachPos);
            }
            break;

        case MovementState::regroup:
            if (GetSpreadRadius() < RegroupRadius / 2.0f) {
                if (m_wasApproaching) {
                    m_moveState = MovementState::approach;
                    gHistory.debug(LogChannel::combat) << SquadName() << " regroup finished, continuing approach" << std::endl;
                } else {
                    m_moveState = MovementState::idle;
                    gHistory.debug(LogChannel::combat) << SquadName() << " regroup finished, idling" << std::endl;
                }
            } else {
                // TODO: Slightly unnecessary to reissue the movement command each step
                IssueMoveCommand(m_regroupPos);
            }
            break;
    }
}

void Squad::IssueMoveCommand(const sc2::Point2D& position) {
    bool needToWait = false;

    // Unsiege/unburrow units that have a stationary state that prevent them from moving
    for (auto& unit : GetUnits()) {
        switch (unit->unit_type.ToType()) {
            case sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
                gAPI->action().Cast(unit, sc2::ABILITY_ID::MORPH_UNSIEGE);
                needToWait = true;
                break;
            case sc2::UNIT_TYPEID::TERRAN_LIBERATORAG:
                gAPI->action().Cast(unit, sc2::ABILITY_ID::MORPH_LIBERATORAAMODE);
                needToWait = true;
                break;
            case sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED:
                gAPI->action().Cast(unit, sc2::ABILITY_ID::BURROWUP);
                needToWait = true;
                break;
            default:
                break;
        }
    }

    if (!needToWait)
        gAPI->action().MoveTo(GetUnits(), position);
}

int Squad::Size() const {
    return static_cast<int>(m_units.size());
}

const sc2::Point2D& Squad::GetAttackMovePoint() const {
    return m_enemyCenter;
}
