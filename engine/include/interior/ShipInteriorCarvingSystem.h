#pragma once

#include "content/ShipyardModuleSystem.h"
#include "interior/ShipInteriorSystem.h"
#include "interior/ShipModuleInteriorLinkSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "world/WorldScaleAuthoritySystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct InteriorCarvedVolume {
    std::size_t moduleIndex = 0;
    std::string moduleId;
    InteriorRoomType roomType = InteriorRoomType::Corridor;
    ExteriorInteriorCapability capability = ExteriorInteriorCapability::None;
    Vector3 center{};
    Vector3 halfExtents{};
    float yawDegrees = 0.0f;
    float pitchDegrees = 0.0f;
    float rollDegrees = 0.0f;
    int deck = 0;
    bool walkable = false;
    bool pressureCapable = false;
};

struct InteriorCarvedPortal {
    std::size_t moduleA = 0;
    std::size_t moduleB = 0;
    Vector3 center{};
    InteriorPortalKind kind = InteriorPortalKind::Door;
    bool walkable = false;
};

struct InteriorExclusionVolume {
    std::size_t moduleIndex = 0;
    std::string moduleId;
    Vector3 center{};
    Vector3 halfExtents{};
    std::string reason;
};

struct ShipInteriorCarvePlan {
    bool valid = false;
    int deckCount = 1;
    std::size_t walkableModuleCount = 0;
    std::size_t connectedWalkableCount = 0;
    std::vector<InteriorCarvedVolume> volumes;
    std::vector<InteriorCarvedPortal> portals;
    std::vector<InteriorExclusionVolume> exclusions;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Derives the pressure/walkable hull from the *final assembled ship recipe*.
/// This is deliberately between exterior assembly and room furnishing: it
/// carves inset walkable volumes from transformed modules, excludes propulsion
/// / weapons / wings / surface machinery, stitches attachment portals and
/// verifies that the walkable graph is one cohesive interior.
class ShipInteriorCarvingSystem {
public:
    static ShipInteriorCarvePlan Carve(const std::vector<ShipyardModuleRecord>& catalog,
                                       const ProceduralShipVisualRecipe& recipe,
                                       const WorldScaleProfile& scale = WorldScaleAuthoritySystem::DefaultProfile());
};

} // namespace subspace
