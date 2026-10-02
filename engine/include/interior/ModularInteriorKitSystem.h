#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class InteriorModuleKind {
    Floor, Wall, Door, CorridorStraight, CorridorCorner, CorridorTJunction,
    CorridorCross, RoomShell, Airlock, MachineryMount, PropMount, Ceiling,
    Window, Stair, Ladder, Bulkhead, DoorFrame, CorridorEnd, Console,
    Furniture, Pipe, Cable, Light
};

enum class InteriorSocketDirection { North, East, South, West, Up, Down };

struct InteriorSnapSocket {
    std::string id;
    InteriorSocketDirection direction = InteriorSocketDirection::North;
    int gridX = 0;
    int gridY = 0;
    int deckOffset = 0;
    std::string compatibility = "interior";

    // R178 fine-placement metadata. Grid coordinates remain authoritative for
    // macro layout; local metre offsets allow imported doors/pipes/details to
    // retain 0.25 m snap fidelity.
    double localX = 0.0;
    double localY = 0.0;
    double localZ = 0.0;
    double snapIncrementMeters = 0.25;
    std::string socketClass = "structural";
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
    // Historical aggregate-initializer order remains intact through sockets.
    std::vector<InteriorSnapSocket> sockets;
    std::string sourcePackId;
    std::string sourceObjectName;
    bool walkableSurface = false;
    bool portalCapable = false;

    // Canonical import/hydration contract.
    double widthMeters = 0.0;
    double lengthMeters = 0.0;
    double heightMeters = 0.0;
    double fineSnapMeters = 0.25;
    double structuralGridMeters = 1.0;
    double planningCellMeters = 2.0;
    double minimumTraversalClearanceMeters = 0.75;
    std::string collisionProfile = "solid";
    std::string visualStyleFamily = "industrial_modular";
    std::vector<std::string> compatibleContexts;
    std::vector<std::string> tags;
    std::string sourceLicense;
    std::string sourceAuthor;
    std::string sourceUri;
    bool requiresHydration = false;
    bool supportsSkins = true;
    bool supportsDamageStates = true;
};

struct InteriorKitValidation {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct InteriorModuleKit {
    std::string kitId;
    double cellSizeMeters = 2.0;
    double deckHeightMeters = 3.0;
    std::vector<InteriorModuleAssetDef> modules;

    // Three-level authoring grid: room planning -> structure -> detail.
    double planningCellMeters = 2.0;
    double structuralGridMeters = 1.0;
    double fineSnapMeters = 0.25;
    std::string family = "industrial_modular";
    std::string sourceLicense;
    std::string sourceAuthor;
    bool hydrated = false;
};

/// Normalizes imported modular art into one project-owned snapping/gameplay
/// contract. Source meshes provide geometry and provenance, never gameplay
/// authority.
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
