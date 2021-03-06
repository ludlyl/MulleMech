
#include "CombatCommander.h"
#include "BuildingPlacer.h"
#include "Historican.h"
#include "Hub.h"
#include "IntelligenceHolder.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Timer.h"

#include <sc2api/sc2_common.h>

#include <algorithm>

CombatCommander::CombatCommander() :
    m_mainSquad(std::make_shared<OffenseSquad>()),
    m_playStyle(PlayStyle::normal),
    m_changedPlayStyle(true)
{

}

void CombatCommander::OnStep(Builder*){
    for (auto& squad : m_defenseSquads)
        squad.OnStep();
    
    m_mainSquad->OnStep();

    // Harass squad
    if (!m_harassSquad.IsSent() && m_harassSquad.Size() >= HarassOnCount)
        m_harassSquad.Send();
    m_harassSquad.OnStep();

    // Reinforce squads
    for (auto itr = m_reinforceSquads.begin(); itr != m_reinforceSquads.end(); ) {
        itr->OnStep();
        if (itr->IsSent() && itr->IsTaskFinished()) {
            m_mainSquad->Absorb(*itr);
            itr = m_reinforceSquads.erase(itr);
        } else {
            if (!itr->IsSent() && itr->Size() >= ReinforceOnCount)
                itr->Send();
            ++itr;
        }
    }

    PlayStyle newPlayStyle = gReasoner->GetPlayStyle();
    if (newPlayStyle != m_playStyle) {
        m_changedPlayStyle = true;
        m_playStyle = newPlayStyle;
    }

    if (m_playStyle != PlayStyle::all_in)
        DefenseCheck();

    switch(m_playStyle) {
        case PlayStyle::normal : PlayNormal();
            break;
        case PlayStyle::all_in : PlayAllIn();
            break;
        case PlayStyle::offensive : PlayOffensive();
            break;
        case PlayStyle::defensive : PlayDefensive();
            break;
        case PlayStyle::very_defensive : PlayVeryDefensive();
            break;
        case PlayStyle::greedy : PlayGreedy();
            break;
        case PlayStyle::scout : PlayScout();
            break;

    }

    m_changedPlayStyle = false;
}

void CombatCommander::PlayNormal(){
    if (m_mainSquad->IsTaskFinished() && m_mainSquad->Size() > 0 && gAPI->observer().GetFoodUsed() >= AttackOnSupply) {
        UpdateMainAttackTarget();
        m_mainSquad->TakeOver(m_mainAttackTarget);
    }
}

void CombatCommander::PlayAllIn(){
    if (m_mainSquad->IsTaskFinished() && m_mainSquad->Size() > 0) {
        // Abandon defense and harass, then attack with all we got
        for (auto& def : m_defenseSquads)
            m_mainSquad->Absorb(def);
        m_defenseSquads.clear();
        if (!m_harassSquad.IsSent())
            m_mainSquad->Absorb(m_harassSquad);
        UpdateMainAttackTarget();
        m_mainSquad->TakeOver(m_mainAttackTarget);
    }
}

void CombatCommander::PlayOffensive(){ // TODO
    PlayNormal();
}

void CombatCommander::PlayDefensive(){ // TODO
    if(m_changedPlayStyle){
        m_mainSquad->AbortTakeOver();
        gAPI->action().MoveTo(m_mainSquad->GetUnits(), GetArmyIdlePosition());
        if (!m_harassSquad.IsSent())
            m_mainSquad->Absorb(m_harassSquad);
    }
}

void CombatCommander::PlayVeryDefensive(){ // TODO
    PlayDefensive();
}

void CombatCommander::PlayGreedy(){ // TODO
    PlayNormal();
}

void CombatCommander::PlayScout(){ // TODO
    PlayNormal();
}

void CombatCommander::UpdateMainAttackTarget() {
    unsigned building_last_seen_step_limit = static_cast<unsigned >(
            std::max(0.f, gAPI->observer().GetGameLoop() - AttackTargetBuildingLastSeenInThreshold * API::StepsPerSecond));
    Expansions expos = gIntelligenceHolder->GetKnownEnemyExpansions();
    if (!expos.empty()) {
        m_mainAttackTarget = expos.back()->town_hall_location;
    // Does gIntelligenceHolder hold enemy units that has been revealed (i.e. is OnUnitEnterVision called on them?)
    } else if (auto enemy_buildings = gIntelligenceHolder->GetEnemyUnits(IsBuilding(), building_last_seen_step_limit); !enemy_buildings.empty()) {
        m_mainAttackTarget = enemy_buildings.front()->pos;
    } else {
        if (m_attackTargets.empty()) {
            m_attackTargets = GetListOfAttackPoints();
        }
        if (!m_attackTargets.empty()) {
            m_mainAttackTarget = m_attackTargets.back();
            m_attackTargets.pop_back();
        }
    }
}

std::vector<sc2::Point2D> CombatCommander::GetListOfAttackPoints() {
    // Get a ground unit in the main squad
    // TODO: Support if we only have air units (just using an air unit here is not enough)
    Unit* ground_unit = nullptr;
    for (auto& unit : m_mainSquad->GetUnits()) {
        if (!unit->is_flying) {
            ground_unit = unit;
        }
    }
    if (!ground_unit) {
        return {};
    }

    Timer clock;
    clock.Start();

    std::vector<sc2::Point2D> points;
    sc2::Point2D point;
    int mapHeightLimit = gAPI->observer().GameInfo().height - ApproximateUnitVisionRadius;
    int mapWidthLimit = gAPI->observer().GameInfo().width - ApproximateUnitVisionRadius;
    for (int x = ApproximateUnitVisionRadius; x < mapWidthLimit; x+= ApproximateUnitVisionRadius) {
        for (int y = ApproximateUnitVisionRadius; y < mapHeightLimit; y+= ApproximateUnitVisionRadius) {
             point.x = x;
             point.y = y;
             points.push_back(point);
        }
    }
    RemovePointsUnreachableByUnit(ground_unit, points);

    auto duration = clock.Finish();
    gHistory.info(LogChannel::combat) << "Calculate list of map (attack) points took: " << duration << " ms" << std::endl;

    return points;
}

std::vector<Units> CombatCommander::GroupEnemiesInBase() {
    auto expansions = gHub->GetOurExpansions();
    if (expansions.empty())
        return {};

    auto ourBuildings = gAPI->observer().GetUnits(IsBuilding(), sc2::Unit::Alliance::Self);

    // Consider defense in regards to every building we have, making a perimeter circle
    // for our base does not work in the general case
    Units enemyUnits = gAPI->observer().GetUnits([&ourBuildings](auto enemy) {
        return sc2::DistanceSquared2D(enemy->pos, ourBuildings.GetClosestUnit(enemy->pos)->pos) <=
                SearchEnemyPadding * SearchEnemyPadding;
    }, sc2::Unit::Alliance::Enemy);

    // Setup base to group map
    std::map<Expansion*, Units&> baseToGroupMap;
    std::vector<Units> enemyGroups;
    enemyGroups.resize(expansions.size());
    for (std::size_t i = 0; i < expansions.size(); ++i) {
        baseToGroupMap.emplace(expansions[i].get(), enemyGroups[i]);
    }

    // Split enemies up into groups based on which base they're attacking
    for (auto& unit : enemyUnits) {
        std::nth_element(expansions.begin(), expansions.begin(), expansions.end(), [&unit](auto& a, auto& b) {
            return sc2::DistanceSquared2D(unit->pos, a->town_hall_location) < sc2::DistanceSquared2D(unit->pos, b->town_hall_location);
        });
        baseToGroupMap.at(expansions[0].get()).push_back(unit);
    }

    // Remove any empty group
    auto itr = std::remove_if(enemyGroups.begin(), enemyGroups.end(), [](auto& g) { return g.empty(); });
    enemyGroups.erase(itr, enemyGroups.end());

    return enemyGroups;
}

void CombatCommander::DefenseCheck() {
    for (auto itr = m_defenseSquads.begin(); itr != m_defenseSquads.end(); ) {
        if (itr->IsTaskFinished()) {
            gHistory.info(LogChannel::combat) << "Defense Squad task finished, re-merging " <<
                                              itr->GetUnits().size() << " units to main squad" << std::endl;
            m_mainSquad->Absorb(*itr);
            itr = m_defenseSquads.erase(itr);
        }
        else
            ++itr;
    }

    auto enemyGroups = GroupEnemiesInBase();
    if (enemyGroups.empty())
        return;

    for (auto& group : enemyGroups) {
        bool dealtWith = false;

        // Update enemy list if we have a squad dealing with them already
        for (auto& squad : m_defenseSquads) {
            if (squad.UpdateEnemies(group)) {
                dealtWith = true;
                break;
            }
        }

        // Need a new squad to deal with this enemy group?
        if (!dealtWith) {
            auto defenders = GenerateDefenseFor(Units(), group);
            if (!defenders.empty())
                m_defenseSquads.emplace_back(std::move(defenders), std::move(group));
        }
    }

    // Add more defenders if necessary
    for (auto& defSquad : m_defenseSquads) {
        auto defenders = GenerateDefenseFor(std::move(defSquad.GetUnits()), defSquad.GetEnemies());
        defSquad.SetUnits(std::move(defenders));
    }

    // Move mainsquad to fighting as well if it's just idling
    if (!m_mainSquad->GetUnits().empty() && m_mainSquad->IsTaskFinished()) {
        float ourValue = 0;
        float theirValue = 0;
        for (auto& group : enemyGroups) {
            for (auto& enemy : group) {
                if (IsCombatUnit()(enemy)) {
                    theirValue += enemy->GetValue();
                }
            }
        }
        for (auto& unit : gAPI->observer().GetUnits(IsCombatUnit(), sc2::Unit::Alliance::Self))
            ourValue += unit->GetValue();

        if (ourValue != 0 && theirValue / ourValue >= DefendWithAllValueRatio) {
            m_mainSquad->TakeOver(enemyGroups[0].CalculateCircle().first);
        }
    }
}

void CombatCommander::OnUnitCreated(Unit* unit_){
    if (!IsCombatUnit()(unit_))
        return;

    // Use unit for harass?
    if (!m_harassSquad.IsSent() && m_playStyle != PlayStyle::all_in &&
        m_playStyle != PlayStyle::very_defensive && m_playStyle != PlayStyle::defensive &&
        (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_REAPER || unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_HELLION)) {
        // Keep harass squads homogenous (reapers don't play nice in group with other units due to cliff walk)
        bool add = true;
        for (auto& unit : m_harassSquad.GetUnits()) {
            if (unit->unit_type != unit_->unit_type) {
                add = false;
                break;
            }
        }

        // Add all hellions to harass squad after we get an armory (i.e. can make hellbats) and none before
        if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_HELLION &&
            gAPI->observer().CountUnitType(sc2::UNIT_TYPEID::TERRAN_ARMORY) == 0)
            add = false;

        if (add) {
            m_harassSquad.AddUnit(unit_);
            return;
        }
    }

    // TODO: Save reapers for a future harass squad, they're a hinderance in the main squad
    if (unit_->unit_type == sc2::UNIT_TYPEID::TERRAN_REAPER)
        return;

    // Assign to main squad or reinforce squad
    if (ShouldReinforce(unit_)) {
        // Add to a reinforce squad if main squad is out and about
        for (auto& squad : m_reinforceSquads) {
            if (!squad.IsSent()) {
                squad.AddUnit(unit_);
                return;
            }
        }
        m_reinforceSquads.emplace_back(m_mainSquad);
        m_reinforceSquads.back().AddUnit(unit_);
    } else {
        m_mainSquad->AddUnit(unit_);
        gAPI->action().MoveTo(m_mainSquad->GetUnits(), GetArmyIdlePosition());
    }
}

Units CombatCommander::GenerateDefenseFor(Units defenders, const Units& enemies) {
    int needed_remaining_resources = 0;
    int needed_antiair_resources = 0;
    int our_value = 0;
    int their_value = 0;

    // TODO: We take into account non combat units here too, that can lead to some pretty bad scenarios
    //  (we build a refinery near the enemy and all of a sudden pull our scv:s as we count in his
    //  overlords and buildings in the enemy unit value). Other the other hand we should kill
    //  non combat units close to our base to so just checking IsCombatUnit is not enough of a solution
    // Total resources we need for defense
    for (auto& enemy : enemies) {
        auto value = enemy->GetValue();
        their_value += value;
        if (enemy->is_flying)
            needed_antiair_resources += value;
        else
            needed_remaining_resources += value;
    }

    needed_antiair_resources *= DefenseResourcesOveredo;
    needed_remaining_resources *= DefenseResourcesOveredo;

    // Subtract current defenders
    for (auto& defender : defenders) {
        auto value = defender->GetValue();
        our_value += value;
        if (defender->CanAttackFlying())
            needed_antiair_resources -= value;
        else
            needed_remaining_resources -= value;
    }

    if (needed_antiair_resources <= 0) {
        needed_remaining_resources += needed_antiair_resources;
        if (needed_remaining_resources <= 0)
            return defenders;
    }

    // Grab new units
    AddDefenders(defenders, enemies.CalculateCircle().first, needed_antiair_resources,
        needed_remaining_resources, our_value, their_value);

    return defenders;
}

void CombatCommander::AddDefenders(Units& defenders, const sc2::Point2D& location, int needed_antiair_resources,
    int needed_remaining_resources, int our_value, int their_value) {
    // Prefer close units
    Units sorted_mainsquad = m_mainSquad->GetUnits(); // make a copy for the purpose of sorting
    std::sort(sorted_mainsquad.begin(), sorted_mainsquad.end(), ClosestToPoint2D(location));

    // Grab anti-air units
    for (auto itr = sorted_mainsquad.begin(); itr != sorted_mainsquad.end(); ) {
        if (needed_antiair_resources <= 0)
            break;

        if ((*itr)->CanAttackFlying()) {
            auto value = (*itr)->GetValue();
            needed_antiair_resources -= value;
            our_value += value;
            defenders.push_back(*itr);
            m_mainSquad->RemoveUnit(*itr);
            itr = sorted_mainsquad.erase(itr);
        } else {
            ++itr;
        }
    }

    // Grab any unit
    needed_remaining_resources += needed_antiair_resources;
    for (auto itr = sorted_mainsquad.begin(); itr != sorted_mainsquad.end(); ) {
        if (needed_remaining_resources <= 0)
            break;
        auto value = (*itr)->GetValue();
        needed_remaining_resources -= value;
        our_value += value;
        defenders.push_back(*itr);
        m_mainSquad->RemoveUnit(*itr);
        itr = sorted_mainsquad.erase(itr);
    }

    // Use SCVs as a last resort
    float scv_value_ratio = std::pow(1.1f, our_value / 500.0f) + 0.1f;
    auto scv_pull_value = their_value - our_value * scv_value_ratio;
    while (scv_pull_value > 0) {
        auto worker = GetClosestFreeWorker(location, true);
        if (!worker)
            break;
        scv_pull_value -= worker->GetValue() * scv_value_ratio;
        defenders.push_back(worker);
        worker->SetAsFighter();
    }
}

sc2::Point3D CombatCommander::GetArmyIdlePosition() const {
    // TODO: Calculate a more sensible position of where to keep our army

    auto expansions = gHub->GetOurExpansions();
    if (expansions.empty())
        return gAPI->observer().StartingLocation();

    auto town_hall = expansions.back()->town_hall_location;
    auto direction_vector = town_hall - expansions.back()->center_behind_minerals;
    sc2::Normalize3D(direction_vector);

    return town_hall + direction_vector * IdleDistance;
}

bool CombatCommander::ShouldReinforce(const Unit* unit) const {
    if (!m_mainSquad->IsTaskFinished())
        return true;

    // Assumption: being close to one of our CCs means we're not offensive
    auto ccs = gAPI->observer().GetUnits(IsTownHall(), sc2::Unit::Alliance::Self);
    if (auto cc = ccs.GetClosestUnit(unit->pos))
        return sc2::Distance2D(unit->pos, cc->pos) >= ReinforceSquadDist;

    return sc2::Distance2D(unit->pos, m_mainSquad->GetCenter()) >= ReinforceSquadDist;
}
