#include "ReinforceSquad.h"

#include "Historican.h"

ReinforceSquad::ReinforceSquad(std::shared_ptr<Squad> squad) : m_targetSquad(std::move(squad)), m_sent(false) { }

bool ReinforceSquad::IsTaskFinished() const {
    return m_targetSquad->GetUnits().empty() ||
        Distance2D(m_targetSquad->GetCenter(), GetCenter()) <= m_targetSquad->GetSpreadRadius();
}

void ReinforceSquad::Send() {
    gHistory.info(LogChannel::combat) << SquadName() << " heading out to reinforce!" << std::endl;
    m_sent = true;
}

void ReinforceSquad::Update() {
    if (!m_sent)
        return;

    if (!IsMoving() || Distance2D(m_targetSquad->GetCenter(), GetApproachPoint()) > m_targetSquad->GetSpreadRadius() / 2.0f) {
        Approach(m_targetSquad->GetCenter());
    }
}
