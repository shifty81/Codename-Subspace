#pragma once

#include "construction/AssemblyConstructionSystem.h"
#include "content/ShipyardModuleSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct AssemblyRenderInstanceProduct { std::string elementId; std::string moduleId; BuildTransform transform{}; };
struct AssemblyCollisionProxyProduct { std::string elementId; Double3 center{}; Double3 halfExtents{}; DoubleQuat rotation{}; };
struct AssemblyInteriorVolumeProduct { std::string elementId; Double3 center{}; Double3 halfExtents{}; bool habitable=false; };
struct AssemblyPortalProduct { std::string aElementId; std::string bElementId; Double3 midpoint{}; bool structural=true; };
struct AssemblyNavAnchorProduct { std::string id; Double3 position{}; };
struct AssemblyPropulsionPortProduct { std::string elementId; Double3 position{}; Double3 exhaustDirection{0.0,-1.0,0.0}; double radiusHint=0.1; };

struct AssemblyRuntimeProducts {
    bool valid=false;
    AssemblyCompileSnapshot source{};
    std::vector<AssemblyRenderInstanceProduct> renderInstances;
    std::vector<AssemblyCollisionProxyProduct> collisionProxies;
    std::vector<AssemblyInteriorVolumeProduct> interiorVolumes;
    std::vector<AssemblyPortalProduct> portals;
    std::vector<AssemblyNavAnchorProduct> navAnchors;
    std::vector<AssemblyPropulsionPortProduct> propulsionPorts;
    Double3 boundsMin{};
    Double3 boundsMax{};
    Double3 centerOfMass{};
    std::string fingerprint;
    std::vector<std::string> warnings;
};

/// Deterministic downstream product compiler for canonical assemblies.  This is
/// the bridge point consumed by renderer/collision/interior/nav/runtime layers;
/// those products no longer need to reverse-engineer a legacy visual recipe.
class AssemblyRuntimeProductSystem {
public:
    static AssemblyRuntimeProducts Compile(const AssemblyDefinition& assembly,
                                           const std::vector<ShipyardModuleRecord>& catalog,
                                           const std::vector<std::string>& requiredCapabilities = {});
};

} // namespace subspace
