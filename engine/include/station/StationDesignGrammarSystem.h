#pragma once

#include "station/StationEcologySystem.h"
#include "station/StationKitbashCatalogSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct StationDesignGrammar {
    std::string id;
    StationArchetype archetype = StationArchetype::TradeHub;
    int branchCount = 4;
    int spineSegments = 1;
    float branchLengthScale = 1.0f;
    bool radial = true;
    bool asteroidEmbedded = false;
    std::vector<StationKitbashPieceRole> requiredPieces;
    std::vector<StationKitbashPieceRole> optionalPieces;
};

class StationDesignGrammarSystem {
public:
    static StationDesignGrammar ForArchetype(StationArchetype archetype, bool asteroidEmbedded = false);
    static bool Requires(const StationDesignGrammar& grammar, StationKitbashPieceRole role);
};

} // namespace subspace
