#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class GalaxyStarKind { Red, Orange, Yellow, White, Blue };
struct GalaxySystemRecord { std::uint32_t id=0; std::string name; float x=0,y=0,z=0; GalaxyStarKind star=GalaxyStarKind::Yellow; float security=.5f; float economy=.5f; float resourceRichness=.5f; int populationTier=0; bool discovered=false; bool hasShipyard=false; bool hasAnomaly=false; };
class GalaxyCatalogSystem {
public:
    std::vector<GalaxySystemRecord> Generate(std::uint64_t seed,std::size_t count) const;
    const GalaxySystemRecord* Find(const std::vector<GalaxySystemRecord>& catalog,std::uint32_t id) const;
};

} // namespace subspace
