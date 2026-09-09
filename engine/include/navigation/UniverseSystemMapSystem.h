#pragma once

#include "celestial/OrbitalDynamicsSystem.h"
#include <string>
#include <vector>

namespace subspace {

struct UniverseMapNode {
    std::uint64_t id=0;
    std::string name;
    OrbitalBodyKind kind=OrbitalBodyKind::Planet;
    Vector3 currentPosition{};
    std::vector<Vector3> orbitTrack;
    bool dockable=false;
};

struct UniverseSystemMapSnapshot { double simulationSeconds=0.0; std::vector<UniverseMapNode> nodes; };

class UniverseSystemMapSystem {
public:
    UniverseSystemMapSnapshot Build(const std::vector<OrbitalBodyRecord>& bodies,double simulationSeconds,int orbitSamples=48) const;
};

} // namespace subspace
