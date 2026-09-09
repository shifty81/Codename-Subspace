#include "trade_route/IndustrialLogisticsSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

namespace {
bool SameSector(const IndustrialNode& a, const IndustrialNode& b) {
    return a.sectorX == b.sectorX && a.sectorY == b.sectorY && a.sectorZ == b.sectorZ;
}

bool IsOrbitalAuthority(LogisticsNodeType type) {
    return type == LogisticsNodeType::OrbitalHub ||
           type == LogisticsNodeType::StationDepot ||
           type == LogisticsNodeType::SectorRelay;
}

bool IsPlanetaryAuthority(LogisticsNodeType type) {
    switch (type) {
        case LogisticsNodeType::PlanetaryExtractor:
        case LogisticsNodeType::PlanetaryProcessor:
        case LogisticsNodeType::PlanetaryWarehouse:
        case LogisticsNodeType::SpaceElevatorTerminal:
        case LogisticsNodeType::SurfaceExtractor:
        case LogisticsNodeType::SurfaceFactory:
        case LogisticsNodeType::SurfaceWarehouse:
        case LogisticsNodeType::SurfaceLaunchPad:
            return true;
        default:
            return false;
    }
}

bool IsElevatorAuthority(LogisticsNodeType type) {
    return type == LogisticsNodeType::SpaceElevatorTerminal ||
           type == LogisticsNodeType::SurfaceLaunchPad; // legacy alias semantics
}
} // namespace

bool IndustrialLogisticsNetwork::AddOrUpdateNode(const IndustrialNode& node) {
    if (node.nodeId.empty()) return false;
    _nodes[node.nodeId] = node;
    return true;
}

LogisticsValidationResult IndustrialLogisticsNetwork::AddOrUpdateRoute(const IndustrialRoute& route) {
    auto result = ValidateRoute(route);
    if (!result.valid) return result;
    _routes[route.routeId] = route;
    return result;
}

const IndustrialNode* IndustrialLogisticsNetwork::GetNode(const std::string& nodeId) const {
    auto it = _nodes.find(nodeId);
    return it == _nodes.end() ? nullptr : &it->second;
}

const IndustrialRoute* IndustrialLogisticsNetwork::GetRoute(const std::string& routeId) const {
    auto it = _routes.find(routeId);
    return it == _routes.end() ? nullptr : &it->second;
}

LogisticsValidationResult IndustrialLogisticsNetwork::ValidateRoute(const IndustrialRoute& route) const {
    if (route.routeId.empty()) return {false, "Route id is required."};
    if (route.fromNodeId == route.toNodeId) return {false, "Route endpoints must differ."};
    const auto* from = GetNode(route.fromNodeId);
    const auto* to = GetNode(route.toNodeId);
    if (!from || !to) return {false, "Both route endpoints must exist."};
    if (!from->online || !to->online) return {false, "Both route endpoints must be online."};
    if (route.capacityPerTrip < 0.0f || route.travelSeconds <= 0.0f)
        return {false, "Route capacity/travel time are invalid."};

    switch (route.scope) {
        case LogisticsRouteScope::Surface:
            if (from->planetId.empty() || from->planetId != to->planetId)
                return {false, "Surface routes must remain on one planet."};
            if (!SameSector(*from, *to))
                return {false, "Surface endpoints must share a sector."};
            break;

        case LogisticsRouteScope::SurfaceToOrbit: {
            if (!SameSector(*from, *to))
                return {false, "Planet-to-orbit routes must stay in one sector."};

            const bool fromPlanet = !from->planetId.empty() && IsPlanetaryAuthority(from->type);
            const bool toPlanet = !to->planetId.empty() && IsPlanetaryAuthority(to->type);
            const bool fromOrbital = IsOrbitalAuthority(from->type);
            const bool toOrbital = IsOrbitalAuthority(to->type);

            // Current content should route Planetary Manufacturing through a
            // completed SpaceElevatorTerminal. Legacy SurfaceLaunchPad routes
            // remain accepted so older saves/tests do not regress.
            const bool validDirection =
                (fromPlanet && IsElevatorAuthority(from->type) && toOrbital) ||
                (toPlanet && IsElevatorAuthority(to->type) && fromOrbital) ||
                (fromPlanet && toOrbital && from->type == LogisticsNodeType::SurfaceLaunchPad) ||
                (toPlanet && fromOrbital && to->type == LogisticsNodeType::SurfaceLaunchPad);
            if (!validDirection)
                return {false, "Planet-to-orbit logistics requires a completed space-elevator terminal and an orbital authority."};
            break;
        }

        case LogisticsRouteScope::InSystem:
            if (!SameSector(*from, *to))
                return {false, "In-system logistics endpoints must share a sector."};
            if (!IsOrbitalAuthority(from->type) || !IsOrbitalAuthority(to->type))
                return {false, "In-system logistics requires orbital/station endpoints."};
            break;

        case LogisticsRouteScope::InterSector:
            if (SameSector(*from, *to))
                return {false, "Inter-sector logistics requires different sectors."};
            if (!IsOrbitalAuthority(from->type) || !IsOrbitalAuthority(to->type))
                return {false, "Inter-sector logistics requires orbital/station/relay endpoints."};
            if (route.assignedShipId.empty())
                return {false, "Inter-sector logistics requires an assigned hauler."};
            break;
    }

    return {true, {}};
}

bool IndustrialLogisticsNetwork::IsProductionChainComplete(const ProductionChain& chain) const {
    if (chain.orderedNodeIds.size() < 2) return false;

    for (const auto& id : chain.orderedNodeIds) {
        const auto* node = GetNode(id);
        if (!node || !node->online) return false;
    }

    for (size_t i = 1; i < chain.orderedNodeIds.size(); ++i) {
        const std::string& fromId = chain.orderedNodeIds[i - 1];
        const std::string& toId = chain.orderedNodeIds[i];
        bool linked = false;
        for (const auto& [routeId, route] : _routes) {
            (void)routeId;
            if (!route.enabled) continue;
            if ((route.fromNodeId == fromId && route.toNodeId == toId) ||
                (route.fromNodeId == toId && route.toNodeId == fromId)) {
                linked = true;
                break;
            }
        }
        if (!linked) return false;
    }

    return true;
}

bool IndustrialLogisticsNetwork::HasEnabledRoute(LogisticsRouteScope scope) const {
    for (const auto& [id, route] : _routes) {
        (void)id;
        if (route.enabled && route.scope == scope && ValidateRoute(route).valid) return true;
    }
    return false;
}

int IndustrialLogisticsNetwork::CountOnlineOrbitalHubs() const {
    int count = 0;
    for (const auto& [id, node] : _nodes) {
        (void)id;
        if (node.online && node.type == LogisticsNodeType::OrbitalHub) ++count;
    }
    return count;
}

AutomationTier IndustrialLogisticsNetwork::EvaluateAutomationTier() const {
    if (HasEnabledRoute(LogisticsRouteScope::InterSector))
        return AutomationTier::InterSectorNetwork;

    if (CountOnlineOrbitalHubs() >= 2 && HasEnabledRoute(LogisticsRouteScope::InSystem))
        return AutomationTier::MultiPlanet;

    if (CountOnlineOrbitalHubs() >= 1 && HasEnabledRoute(LogisticsRouteScope::SurfaceToOrbit))
        return AutomationTier::OrbitalExport;

    if (HasEnabledRoute(LogisticsRouteScope::Surface))
        return AutomationTier::LocalSurface;

    return AutomationTier::Manual;
}

bool IndustrialLogisticsNetwork::CanAutomate(const IndustrialRoute& route) const {
    if (!route.enabled || !ValidateRoute(route).valid) return false;

    const AutomationTier tier = EvaluateAutomationTier();
    switch (route.scope) {
        case LogisticsRouteScope::Surface:
            return tier >= AutomationTier::LocalSurface;
        case LogisticsRouteScope::SurfaceToOrbit:
            return tier >= AutomationTier::OrbitalExport;
        case LogisticsRouteScope::InSystem:
            return tier >= AutomationTier::MultiPlanet;
        case LogisticsRouteScope::InterSector:
            return tier >= AutomationTier::InterSectorNetwork;
    }
    return false;
}

TradeRoute IndustrialLogisticsNetwork::BuildTradeRoute(const IndustrialRoute& route) const {
    TradeRoute out;
    const auto validation = ValidateRoute(route);
    if (!validation.valid) return out;

    const auto* from = GetNode(route.fromNodeId);
    const auto* to = GetNode(route.toNodeId);
    if (!from || !to) return out;

    out.routeId = route.routeId;
    out.routeName = route.displayName.empty() ? route.routeId : route.displayName;
    out.isLoop = true;

    TradeWaypoint a;
    a.stationId = from->nodeId;
    a.stationName = from->displayName;
    a.x = static_cast<float>(from->sectorX) * 10000.0f + from->localX;
    a.y = static_cast<float>(from->sectorY) * 10000.0f + from->localY;
    a.z = 0.0f; // authoritative strategic travel remains 2D

    TradeWaypoint b;
    b.stationId = to->nodeId;
    b.stationName = to->displayName;
    b.x = static_cast<float>(to->sectorX) * 10000.0f + to->localX;
    b.y = static_cast<float>(to->sectorY) * 10000.0f + to->localY;
    b.z = 0.0f;

    out.waypoints = {a, b};
    out.totalDistance = out.CalculateDistance();
    return out;
}

} // namespace subspace
