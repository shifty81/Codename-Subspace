#pragma once

#include "interior/ShipModuleInteriorLinkSystem.h"
#include "ships/ShipClassRoleSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct ShipInteriorRoomVolume {
    std::size_t moduleIndex = 0;
    std::string moduleId;
    ExteriorInteriorCapability capability = ExteriorInteriorCapability::None;
    int deck = 0;
    int widthCells = 1;
    int lengthCells = 1;
    float cellSizeMeters = 2.0f;
    float deckHeightMeters = 3.0f;
    bool walkable = false;
};

struct ShipInteriorPortalLink {
    std::size_t moduleA = 0;
    std::size_t moduleB = 0;
    std::string portalA;
    std::string portalB;
    InteriorPortalKind kind = InteriorPortalKind::Door;
    bool walkable = false;
};

struct GeneratedShipInteriorProgram {
    bool valid = false;
    ShipClass shipClass = ShipClass::Frigate;
    int targetDeckCount = 1;
    std::vector<ShipInteriorRoomVolume> rooms;
    std::vector<ShipInteriorPortalLink> portals;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

/// Converts the exterior/interior binding plan into an editable interior-volume
/// program. Every walkable exterior module owns room/deck volume inside its own
/// bounds; exterior attachment edges become candidate interior portals.
class ShipInteriorAuthoringSystem {
public:
    static int TargetDeckCount(ShipClass shipClass);
    static GeneratedShipInteriorProgram Generate(const ShipInteriorConnectionPlan& plan,
                                                  ShipClass shipClass,
                                                  float cellSizeMeters = 2.0f,
                                                  float deckHeightMeters = 3.0f);
};

} // namespace subspace
