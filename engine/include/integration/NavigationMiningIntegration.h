#pragma once

#include "navigation/SystemMapSystem.h"
#include "navigation/SystemNavigationSystem.h"
#include "navigation/VectorTravelSystem.h"
#include "procedural/RegionStreamingSystem.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "scanning/DeepExplorationSystem.h"
#include "navigation/AnomalousSpaceSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct LocalCelestialScene {
    std::size_t dominantPlanet = static_cast<std::size_t>(-1);
    std::vector<std::size_t> majorDiscs;
    double dominantScreenScale = 0.0;
};

struct BeltOperationState {
    BeltMacroRegion belt;
    RegionCellKey cell;
    StreamedRegionCell materialized;
    std::uint64_t minedNodes = 0;
    double recoveredRichness = 0.0;
};

/// Pass271-275 and exploration-route integration. System intelligence,
/// Vector travel and effectively enormous streamed mining regions share one
/// navigation path instead of separate UI-owned state.
class NavigationMiningIntegration {
public:
    SystemMapSnapshot BuildMapAndNavigation(const GalaxySector& sector,
                                            SystemNavigationSystem& navigation) const;
    bool AddResolvedDiscovery(SystemMapSnapshot& map,SystemNavigationSystem& navigation,
                              std::uint64_t id,const std::string& label,
                              const AstronomicalPosition& position,double hazard,
                              bool bookmark=true) const;
    bool AddTemporaryTear(SystemMapSnapshot& map,SystemNavigationSystem& navigation,
                          std::uint64_t id,const std::string& label,
                          const AstronomicalPosition& position,double hazard) const;
    WarpPlan PlanVector(const SystemNavigationSystem& navigation,
                        const AstronomicalPosition& from,std::uint64_t destination,
                        double driveMetersPerSecond,double fuel) const;
    LocalCelestialScene EvaluateLocalCelestials(const GalaxySector& sector,
                                                const Vector3& playerWorld) const;
    BeltOperationState EnterBelt(const BeltMacroRegion& belt,RegionCellKey cell,
                                 RegionStreamingSystem& streaming) const;
    bool MineNode(BeltOperationState& state,std::uint64_t nodeId,
                  RegionStreamingSystem& streaming) const;
    RegionCellKey NextUnsurveyed(const BeltOperationState& state,
                                 RegionStreamingSystem& streaming,std::uint64_t salt=0) const;
};

} // namespace subspace
