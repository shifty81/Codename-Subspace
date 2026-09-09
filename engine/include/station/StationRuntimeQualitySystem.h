#pragma once
#include "content/ShipyardModuleSystem.h"
#include "station/StationEcologySystem.h"
#include "station/StationKitbashVisualSystem.h"
#include <cstdint>
#include <string>
#include <vector>
namespace subspace {
struct StationRuntimeQualityReport {
    bool certified=false;
    bool primitiveFallbackAllowed=false;
    bool connectedGraph=false;
    bool dockConnected=false;
    int moduleCount=0;
    int structuralModules=0;
    int commandModules=0;
    int serviceModules=0;
    int sensorModules=0;
    int hardpointModules=0;
    int overlapConflicts=0;
    int missingRequiredPieces=0;
    float maxConnectionGap=0.0f;
    std::vector<std::string> issues;
};
class StationRuntimeQualitySystem {
public:
    StationRuntimeQualityReport Audit(const std::vector<ShipyardModuleRecord>& catalog,
                                      StationArchetype archetype,
                                      std::uint64_t seed,
                                      bool asteroidEmbedded=false) const;
};
}
