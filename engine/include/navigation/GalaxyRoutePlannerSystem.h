#pragma once

#include "navigation/GalaxyCatalogSystem.h"
#include <cstdint>
#include <vector>

namespace subspace {

enum class GalaxyRouteMode { Fastest, Safest, LowestFuel, Logistics, Exploration };
struct GalaxyRouteRequest { std::uint32_t start=0,end=0; GalaxyRouteMode mode=GalaxyRouteMode::Fastest; float jumpRange=700.0f; float fuelPerDistance=.001f; float minimumSecurity=0.0f; };
struct GalaxyRouteResult { bool valid=false; std::vector<std::uint32_t> systems; float distance=0; float fuel=0; float risk=0; };
class GalaxyRoutePlannerSystem {
public:
    GalaxyRouteResult Plan(const std::vector<GalaxySystemRecord>& catalog,const GalaxyRouteRequest& request) const;
};

} // namespace subspace
