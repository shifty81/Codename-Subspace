#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class InteriorModuleKind {
    Floor,
    Wall,
    Door,
    CorridorStraight,
    CorridorCorner,
    CorridorTJunction,
    CorridorCross,
    RoomShell,
    Airlock,
    MachineryMount,
    PropMount,
    Ceiling,
    Window,
    Stair,
    Ladder,
    Bulkhead,
    DoorFrame,
    CorridorEnd,
    Console,
    Furniture,
    Pipe,
    Cable,
    Light
};

enum class InteriorSocketDirection { North, East, South, West, Up, Down };

struct InteriorSnapSocket {
    std::string id;
    InteriorSocketDirection direction = InteriorSocketDirection::North;
    int gridX = 0;
    int gridY = 0;
    int deckOffset = 0;
    std::string compatibility = "interior";
};

struct InteriorModuleAssetDef {
    std::string assetId;
    std::string sourceAssetId;
    InteriorModuleKind kind = InteriorModuleKind::Floor;
    int widthCells = 1;
    int heightCells = 1;
    double cellSizeMeters = 2.0;
    double deckHeightMeters = 3.0;
    bool collisionEnabled = true;
    bool gameplayMount = false;
    // Keep sockets in the original aggregate-initializer position used by the
    // Pass316+ runtime/test contract. New provenance and semantic fields are
    // append-only so older authored data remains source-compatible.
    std::vector<InteriorSnapSocket> sockets;
    std::string sourcePackId;
    std::string sourceObjectName;
    bool walkableSurface = false;
    bool portalCapable = false;
};

struct InteriorKitValidation {
    bool valid = false;
    std::vector<std::string> errors;
};

struct InteriorModuleKit {
    std::string kitId;
    double cellSizeMeters = 2.0;
    double deckHeightMeters = 3.0;
    std::vector<InteriorModuleAssetDef> modules;
};

/// Pass319 normalizes imported modular art into Subspace-owned snapping and
/// gameplay contracts. The source mesh never decides room semantics directly.
class ModularInteriorKitSystem {
public:
    InteriorModuleAssetDef Normalize(InteriorModuleAssetDef module, const InteriorModuleKit& kit) const;
    InteriorKitValidation Validate(const InteriorModuleKit& kit) const;
    const InteriorModuleAssetDef* FindByKind(const InteriorModuleKit& kit, InteriorModuleKind kind) const;
    InteriorModuleKind ClassifySourceName(const std::string& sourceObjectName) const;
    InteriorModuleAssetDef BuildImportedModule(const std::string& assetId,
                                               const std::string& sourcePackId,
                                               const std::string& sourceObjectName,
                                               const InteriorModuleKit& kit) const;
};

} // namespace subspace
