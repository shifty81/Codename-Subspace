#pragma once

#include "station/StationKitbashVisualSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct StationDesignDna {
    std::string id;
    std::string grammarId;
    StationArchetype archetype = StationArchetype::TradeHub;
    int moduleCount = 0;
    int attachmentCount = 0;
    int branchLikeModules = 0;
    float halfWidth = 0.0f;
    float halfLength = 0.0f;
    float halfHeight = 0.0f;
    float radiality = 0.0f;
    float symmetry = 0.0f;
    float structuralRatio = 0.0f;
    std::vector<std::string> pieceVocabulary;
};

struct StationDesignExemplar {
    std::string name;
    StationDesignDna dna;
    float weight = 1.0f;
    bool approved = true;
};

struct StationDesignFamily {
    std::string id;
    StationArchetype archetype = StationArchetype::TradeHub;
    int exemplarCount = 0;
    float averageRadiality = 0.0f;
    float averageSymmetry = 0.0f;
    float averageStructuralRatio = 0.0f;
    std::vector<std::string> vocabulary;
};

class StationDesignDnaSystem {
public:
    static StationDesignDna Extract(const StationKitbashVisualRecipe& recipe,
                                    const std::vector<ShipyardModuleRecord>& catalog);
    static StationDesignFamily CompileFamily(const std::string& id,
                                             StationArchetype archetype,
                                             const std::vector<StationDesignExemplar>& exemplars);
};

} // namespace subspace
