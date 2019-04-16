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

    // Get enemy base location if one has been made; index: 0=>main base, 1=>natural, etc
    std::shared_ptr<Expansion> GetEnemyBase(std::size_t index) const;

    std::shared_ptr<Expansion> GetLatestEnemyBase() const;

    bool EnemyHasBase(std::size_t index) const;

    // Note down where enemy main base is
    void MarkEnemyMainBase(const sc2::Point2D& point);

    // Note down where to find an enemy base
    // Note: The enemies main base has to be marked before any expansion can be marked
    void MarkEnemyExpansion(const sc2::Point2D& point);

    // Get all enemy units that we currently have intel on (dead units are cleared in the Update function)
    const Units& GetEnemyUnits() const;

    // Returns a copy
    Units GetEnemyUnits(unsigned int lastSeenByGameLoop) const;

private:
    void SaveEnemyBaseLocation(Unit* unit);

    std::vector<std::shared_ptr<Expansion>> m_enemyBases;
    Units m_enemyUnits;
};

extern std::unique_ptr<IntelligenceHolder> gIntelligenceHolder;
