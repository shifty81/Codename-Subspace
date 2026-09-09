#pragma once

#include "content/ShipyardModuleSystem.h"
#include "interior/ModularInteriorKitSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "world/WorldScaleAuthoritySystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ExteriorInteriorCapability {
    None,
    WalkableRoom,
    Corridor,
    Cockpit,
    Bridge,
    Engineering,
    Cargo,
    Habitation,
    Airlock,
    Hangar,
    ServiceAccess
};

enum class InteriorPortalKind { OpenPassage, Door, Airlock, Hatch, ServiceHatch, SealedBoundary };

struct ShipModuleInteriorPortal {
    std::string id;
    std::string exteriorSocketName;
    InteriorPortalKind kind = InteriorPortalKind::Door;
    Vector3 localPosition{};
    Vector3 localDirection{0.0f,1.0f,0.0f};
    float widthMeters = 1.0f;
    float heightMeters = 2.2f;
};

struct ShipModuleInteriorBinding {
    std::string moduleId;
    ExteriorInteriorCapability capability = ExteriorInteriorCapability::None;
    int deckCount = 1;
    int footprintWidthCells = 1;
    int footprintLengthCells = 1;
    bool walkable = false;
    bool interactionOnly = false;
    std::string preferredInteriorKitId;
    std::vector<ShipModuleInteriorPortal> portals;
};

struct InteriorConnectionEdge {
    std::size_t moduleA = 0;
    std::size_t moduleB = 0;
    std::string portalA;
    std::string portalB;
    InteriorPortalKind kind = InteriorPortalKind::Door;
    bool walkable = false;
    std::string status;
};

struct ShipInteriorConnectionPlan {
    bool valid = false;
    std::vector<ShipModuleInteriorBinding> bindings;
    std::vector<InteriorConnectionEdge> edges;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

class ShipModuleInteriorLinkSystem {
public:
    static const char* CapabilityName(ExteriorInteriorCapability capability);
    static ShipModuleInteriorBinding InferBinding(const ShipyardModuleRecord& module,
                                                  const WorldScaleProfile& scale);
    static ShipInteriorConnectionPlan BuildPlan(const std::vector<ShipyardModuleRecord>& catalog,
                                                const ProceduralShipVisualRecipe& recipe,
                                                const WorldScaleProfile& scale);
    static bool CanCreateWalkableConnection(const ShipModuleInteriorBinding& a,
                                            const ShipModuleInteriorBinding& b);
};

} // namespace subspace
