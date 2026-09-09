#pragma once

#include "procedural/GalaxyGenerator.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class SystemMapNodeKind { Star, Planet, Moon, Station, OrbitalHub, Belt, Salvage, Signature, TradeLane, DeepSpace };

struct SystemMapNode {
    std::uint64_t id = 0;
    SystemMapNodeKind kind = SystemMapNodeKind::DeepSpace;
    std::string label;
    SectorPosition position;
    bool known = true;
    bool warpable = true;
    float hazard = 0.0f;
    float strategicRadius = 1.0f;
    float distanceFromOrigin = 0.0f;
    std::string regionLabel;
};

struct SystemMapSnapshot {
    std::string systemName;
    std::vector<SystemMapNode> nodes;
    std::size_t selected = static_cast<std::size_t>(-1);
};

class SystemMapSystem {
public:
    SystemMapSnapshot Build(const GalaxySector& sector) const;
    bool SelectById(SystemMapSnapshot& map, std::uint64_t id) const;
    const SystemMapNode* Selected(const SystemMapSnapshot& map) const;
    std::vector<SystemMapNode> WarpableKnown(const SystemMapSnapshot& map) const;
    float Extent(const SystemMapSnapshot& map) const;
    std::vector<std::string> SelectedSummary(const SystemMapSnapshot& map) const;
    static const char* KindName(SystemMapNodeKind kind);
};

} // namespace subspace
