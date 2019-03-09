#pragma once

#include "core/Map.h"

// Makes assumptions based on reasoning (inferred from memory, etc)
class Reasoner {
    public:
        // Return: Vector of Expansions sorted by likelihood
        std::vector<std::shared_ptr<Expansion>> GetLikelyEnemyExpansions();
};

extern std::unique_ptr<Reasoner> gReasoner;
