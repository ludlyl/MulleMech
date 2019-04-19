#pragma once

#include "core/Map.h"
#include "core/Units.h"

#include <memory>

// Might wanna consider renaming this class to something better
// Things we have learned about our opponent
class IntelligenceHolder {
public:
    // Updates the internal list(s) of enemy units
    void Update();

    // Returns nullptr if the enemy main-base location is unknown
    std::shared_ptr<Expansion> GetEnemyMainBase();

    // Returns a list of our expansions sorted with walking distance to starting location;
    // index: 0=>main base, 1=>natural, etc
    Expansions GetKnownEnemyExpansions() const;

    std::shared_ptr<Expansion> GetLatestKnownEnemyExpansion() const;

    int GetKnownEnemyExpansionCount() const;

    void MarkEnemyExpansion(const sc2::Point2D& pos);
    void MarkEnemyExpansion(Unit* unit);

    // Get all enemy units that we currently have intel on (dead units are cleared in the Update function)
    const Units& GetEnemyUnits() const;

    // Returns a copy
    Units GetEnemyUnits(unsigned int lastSeenByGameLoop) const;

private:
    std::shared_ptr<Expansion> m_enemy_main_base = nullptr;
    Units m_enemyUnits;
};

extern std::unique_ptr<IntelligenceHolder> gIntelligenceHolder;
