#pragma once

#include "interior/ShipInteriorCarvingSystem.h"
#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

// Derived planar surface shared by the runtime interior renderer and
// the on-foot local traversal solver. Source exterior meshes are unchanged;
// never re-enable an original uncut box collider inside the cavity.
struct InteriorShellSurface {
    std::size_t sourceModule = 0;
    int axis = 0;                    // 0=X, 1=Y, 2=Z
    int direction = 1;               // outward from the walkable cavity
    std::array<Vector3,4> corners{}; // metres, ship-local, outward winding
    bool blocksMovement = true;
    bool pressureBoundary = true;
};

struct InteriorShellOpening {
    std::size_t moduleA = 0;
    std::size_t moduleB = 0;
    int axis = 0;
    Vector3 center{};
    float width = 0.0f;
    float height = 0.0f;
    float gap = 0.0f;
    bool exposedByUnion = false;
};

struct InteriorDerivedShell {
    // A fail-closed separate readiness flag; an existing logical carve can be
    // valid while its exact mesh/collision realization is not yet supported.
    bool ready = false;
    std::vector<InteriorShellSurface> surfaces;
    std::vector<InteriorShellOpening> openings;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

class ShipInteriorDerivedShellSystem {
public:
    // Axis-aligned module envelopes only in this first geometry implementation.
    // The source module meshes are never mutated. The returned surface list is
    // offered to native renderer and the traversal solver from one layout plan.
    static InteriorDerivedShell Build(
        const ShipInteriorCarvePlan& carve,
        const WorldScaleProfile& scale = WorldScaleAuthoritySystem::DefaultProfile());
};

} // namespace subspace
