#pragma once

#include "content/ShipyardModuleSystem.h"
#include "station/StationDesignGrammarSystem.h"
#include "station/StationEcologySystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct StationKitbashVisualRecipe {
    std::string identity;
    std::string grammarId;
    std::uint64_t seed = 0;
    StationArchetype archetype = StationArchetype::TradeHub;
    std::vector<VisualModulePlacement> modules;
    std::vector<VisualAssemblyAttachment> attachments;
    std::vector<std::string> logicalPieceIds;
    std::vector<StationKitbashPieceRole> logicalRoles;
    Vector3 primaryDockLocal{};
    Vector3 primaryDockDirection{0.0f,-1.0f,0.0f};
    bool resolved = false;
    bool connectedGraph = false;
    bool usedStructuralConnectors = false;
    bool usedCommandModule = false;
    bool usedServiceModule = false;
    bool usedSensorModule = false;
    bool usedHardpointModule = false;
    bool usedDedicatedDock = false;
    float maxMeasuredGap = 0.0f;
};

/// Pass556+ station visual authority. Stations reuse certified Shipyard geometry
/// through a station-specific logical kitbash and deterministic connection graph.
/// Generated modules never receive unrelated free-floating coordinates: every
/// non-root module is placed relative to a parent and recorded in attachments.
class StationKitbashVisualSystem {
public:
    static StationKitbashVisualRecipe Build(const std::vector<ShipyardModuleRecord>& catalog,
                                            StationArchetype archetype,
                                            std::uint64_t seed,
                                            bool asteroidEmbedded = false);

    static const char* Identity(StationArchetype archetype);
};

} // namespace subspace
