#include "Reasoner.h"
#include "core/Brain.h"
#include "Hub.h"
#include "IntelligenceHolder.h"

PlayStyle Reasoner::CalculatePlayStyle() {
    return PlayStyle::normal;
}

PlayStyle Reasoner::GetPlayStyle() {
    return m_latest_play_style;
}

std::vector<UnitClass> Reasoner::GetNeededUnitClasses() {
    return {};
}

std::vector<std::shared_ptr<Expansion>> Reasoner::GetLikelyEnemyExpansions() {
    auto main = gIntelligenceHolder->GetEnemyBase(0);
    if (!main)
        return std::vector<std::shared_ptr<Expansion>>();

    // Assumption: All neutral expansion locations are possible targets
    std::vector<std::shared_ptr<Expansion>> locations;
    locations.reserve(gHub->GetExpansions().size());
    for (auto& expansion : gHub->GetExpansions()) {
        if (expansion->alliance == sc2::Unit::Alliance::Neutral)
            locations.push_back(expansion);
    }

    // Assumption: The closer a base is to the enemy's main base, the more attractive they'll find it
    std::sort(locations.begin(), locations.end(), [&main](auto& a, auto& b) {
        return main->distanceTo(a) < main->distanceTo(b);
    });

    return locations;
}

std::unique_ptr<Reasoner> gReasoner;
