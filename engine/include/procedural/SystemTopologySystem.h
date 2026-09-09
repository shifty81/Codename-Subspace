#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class TopologySiteType { Stargate, Station, LargeAsteroid, DerelictCluster, ResourceField, FactionAnchor };

struct TopologySite {
    TopologySiteType type = TopologySiteType::ResourceField;
    double x = 0.0;
    double y = 0.0;
    std::string factionId;
    std::string tag;
};

struct SystemTopology {
    std::uint64_t seed = 0;
    int sectorX = 0;
    int sectorY = 0;
    std::vector<TopologySite> sites;
    int gateCount = 0;
};

class SystemTopologySystem {
public:
    SystemTopology Generate(std::uint64_t galaxySeed, int sectorX, int sectorY, double securityRating) const;
};

} // namespace subspace
