#pragma once

#include "core/Map.h"
#include "core/Units.h"
#include "core/API.h"

#include <memory>

// Might wanna consider renaming this class to something better
// Things we have learned about our opponent
class IntelligenceHolder {
public:
    void OnGameStart();

    void OnUnitEnterVision(Unit* unit_);

    void OnUnitDestroyed(Unit* unit_);

    // Returns nullptr if the enemy main-base location is unknown
    std::shared_ptr<Expansion> GetEnemyMainBase();

    void MarkEnemyMainBasePosition(const sc2::Point2D& pos_);

    // Returns a list of our expansions sorted with walking distance to starting location;
    // index: 0=>main base, 1=>natural, etc
    Expansions GetKnownEnemyExpansions() const;

    std::shared_ptr<Expansion> GetLatestKnownEnemyExpansion() const;

    int GetKnownEnemyExpansionCount() const;

    // By ground distance
    std::shared_ptr<Expansion> GetEnemyBaseFurthestFrom(const std::shared_ptr<Expansion>& expansion_);

    void MarkEnemyExpansion(Unit* unit_);

    // Get all enemy units that we currently have intel on (dead units are cleared in the Update function)
    const Units& GetEnemyUnits() const;

    // Returns a copy
    Units GetEnemyUnits(API::Filter filter_) const;
    Units GetEnemyUnits(unsigned int last_seen_by_game_loop_) const;
    Units GetEnemyUnits(API::Filter filter_, unsigned int last_seen_by_game_loop_) const;

private:
    std::shared_ptr<Expansion> m_enemy_main_base = nullptr;
    bool m_enemy_main_base_destroyed = false; // Needed to not "recalculate" which would be the enemies main if it has been destroyed
    Units m_enemy_units;
};

extern std::unique_ptr<IntelligenceHolder> gIntelligenceHolder;
