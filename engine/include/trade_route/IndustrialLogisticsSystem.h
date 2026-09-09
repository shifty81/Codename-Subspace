#pragma once

#include "trade_route/TradeRouteSystem.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class LogisticsNodeType {
    // Current Planetary Manufacturing authority. Planetary routes are
    // strategic logistics links; ships never land on the planet.
    PlanetaryExtractor,
    PlanetaryProcessor,
    PlanetaryWarehouse,
    SpaceElevatorTerminal,

    // Legacy Pass160-163 names retained for serialized/test compatibility.
    // New content should use the Planetary* / SpaceElevatorTerminal names.
    SurfaceExtractor,
    SurfaceFactory,
    SurfaceWarehouse,
    SurfaceLaunchPad,

    OrbitalHub,
    StationDepot,
    SectorRelay
};

enum class LogisticsRouteScope { Surface, SurfaceToOrbit, InSystem, InterSector };

enum class AutomationTier {
    Manual,
    LocalSurface,
    OrbitalExport,
    MultiPlanet,
    InterSectorNetwork
};

struct IndustrialNode {
    std::string nodeId;
    std::string displayName;
    LogisticsNodeType type = LogisticsNodeType::PlanetaryExtractor;
    std::string planetId;
    int sectorX = 0;
    int sectorY = 0;
    int sectorZ = 0;
    float localX = 0.0f;
    float localY = 0.0f;
    float throughputPerMinute = 0.0f;
    float storageCapacity = 0.0f;
    bool online = true;
    std::vector<std::string> inputGoods;
    std::vector<std::string> outputGoods;
};

struct IndustrialRoute {
    std::string routeId;
    std::string displayName;
    std::string fromNodeId;
    std::string toNodeId;
    LogisticsRouteScope scope = LogisticsRouteScope::Surface;
    std::string assignedShipId;
    float capacityPerTrip = 0.0f;
    float travelSeconds = 1.0f;
    float hazardRisk = 0.0f;
    bool enabled = true;
};

struct ProductionChain {
    std::string chainId;
    std::vector<std::string> orderedNodeIds;
    std::string sourceGood;
    std::string finishedGood;
};

struct LogisticsValidationResult {
    bool valid = false;
    std::string reason;
};

/// Shared strategic logistics authority for Planetary Manufacturing ->
/// Space Elevator -> orbital hub -> station/sector automation. Ships do not
/// land on planets. The network does not simulate flight itself; validated
/// route legs can be materialized into the existing TradeRoute system.
class IndustrialLogisticsNetwork {
public:
    bool AddOrUpdateNode(const IndustrialNode& node);
    LogisticsValidationResult AddOrUpdateRoute(const IndustrialRoute& route);

    const IndustrialNode* GetNode(const std::string& nodeId) const;
    const IndustrialRoute* GetRoute(const std::string& routeId) const;

    bool IsProductionChainComplete(const ProductionChain& chain) const;
    AutomationTier EvaluateAutomationTier() const;
    bool CanAutomate(const IndustrialRoute& route) const;

    TradeRoute BuildTradeRoute(const IndustrialRoute& route) const;

    const std::unordered_map<std::string, IndustrialNode>& GetNodes() const { return _nodes; }
    const std::unordered_map<std::string, IndustrialRoute>& GetRoutes() const { return _routes; }

private:
    LogisticsValidationResult ValidateRoute(const IndustrialRoute& route) const;
    bool HasEnabledRoute(LogisticsRouteScope scope) const;
    int CountOnlineOrbitalHubs() const;

    std::unordered_map<std::string, IndustrialNode> _nodes;
    std::unordered_map<std::string, IndustrialRoute> _routes;
};

} // namespace subspace
