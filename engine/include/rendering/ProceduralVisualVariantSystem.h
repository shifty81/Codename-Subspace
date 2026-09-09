#pragma once

#include "rendering/SpaceMaterialSystem.h"
#include "core/Math.h"
#include "ships/FittingSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

/// Bounds-aware description of one authored module after OBJ->runtime axis
/// remapping. halfLength is measured along gameplay/visual +Y (ship forward).
struct VisualModuleSurfaceContact {
    Vector3 point{};
    Vector3 normal{};
    float supportingArea = 0.0f;
    float confidence = 0.0f;
    bool valid = false;
};

struct VisualModuleSource {
    std::string moduleId;
    float halfWidth = 1.0f;
    float halfLength = 1.0f;
    float halfHeight = 1.0f;

    // R3V1: robust authored-geometry contact surfaces. These remain optional so
    // historical/native fixtures keep working; certified Shipyard runtime
    // sources populate them from triangle-area analysis instead of BBox ends.
    VisualModuleSurfaceContact forwardSurface{};
    VisualModuleSurfaceContact aftSurface{};
    VisualModuleSurfaceContact portSurface{};
    VisualModuleSurfaceContact starboardSurface{};
    VisualModuleSurfaceContact dorsalSurface{};
    VisualModuleSurfaceContact ventralSurface{};
};

/// A single placement of one of the existing authored OBJ modules. Procedural
/// visual generation never invents untraceable geometry; it assembles the
/// repository's current module library into deterministic authored-looking
/// recipes that can be regenerated from the same seed on every load.
enum class VisualDetailKind {
    ArmorPlate,
    Fairing,
    StructuralRib,
    Vent,
    Radiator,
    HardpointBase,
    Conduit,
    MaintenanceHatch,
    HeatShield,
    DecalStripe,
    NavigationLight,
    MountBridge,
    StructuralFill,
    TurretSocket
};

struct VisualDetailPlacement {
    VisualDetailKind kind = VisualDetailKind::ArmorPlate;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float sizeZ = 0.1f;
    float yawDegrees = 0.0f;
    float pitchDegrees = 0.0f;
    float rollDegrees = 0.0f;
    SpaceMaterialKind material = SpaceMaterialKind::ShipHull;
    float intensity = 1.0f;
};

struct VisualModulePlacement {
    std::string moduleId;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;
    float yawDegrees = 0.0f;
    SpaceMaterialKind material = SpaceMaterialKind::ShipHull;
    float pitchDegrees = 0.0f;
    float rollDegrees = 0.0f;
    // True geometric handedness mirror for paired authored parts. A mirror is
    // not equivalent to a 180-degree yaw: wings/fins must preserve ship
    // forward while reflecting their flare/sweep across the centerline.
    bool mirrorX = false;
    // Pass655-674: universal construction symmetry supports all three ship-local planes.
    bool mirrorY = false;
    bool mirrorZ = false;
};


struct VisualAssemblyAttachment {
    std::size_t parentModuleIndex = 0;
    std::size_t childModuleIndex = 0;
    std::string parentSocket;
    std::string childSocket;
    float measuredGap = 0.0f;
    bool certified = true;
};

// Compatibility alias: ships and stations now share one visual attachment
// record instead of maintaining parallel graph metadata.
using ShipVisualAttachment = VisualAssemblyAttachment;

struct ShipVisualAnchor {
    std::string id;
    std::string moduleId;
    Vector3 position{};
    bool functional = true;
};

struct ShipVisualHardpoint {
    std::string id;
    Vector3 position{};
    float yawDegrees = 0.0f;
    FittingHardpointSize size = FittingHardpointSize::Small;
    bool turret = true;
};

struct ProceduralShipVisualRecipe {
    std::string recipeId;
    std::string role;
    std::uint32_t seed = 1;
    float widthScale = 1.0f;
    float lengthScale = 1.0f;
    float accentStrength = 0.35f;
    float armorBreakup = 0.35f;
    float negativeSpaceStrength = 0.25f;
    float asymmetryStrength = 0.0f;
    float articulationDegrees = 0.0f;

    // Canonical ship-forward contract. Gameplay/physics remain +Y-forward;
    // this is the visual normalization offset required to make the authored
    // cockpit/bridge face that same direction. Greyoxide source ships are
    // currently 180 degrees opposed to the native +Y gameplay axis.
    float forwardVisualYawDegrees = 0.0f;
    std::string forwardAuthority = "LEGACY";
    int cockpitModuleIndex = -1;

    float qualityScore = 0.0f;
    bool acceptedByArtDirector = false;
    std::string sourceFamily = "SUBSPACE_NATIVE";
    std::string manufacturerFamily;
    std::string cockpitFamily;
    std::string decalCode;

    // Pass615-654: every generated/manual blueprint may retain faction/class/
    // hull-family/role lineage.  Procedural variation is allowed inside this
    // envelope but must not erase the platform identity that produced it.
    std::string factionId;
    std::string shipClassId;
    std::string hullFamilyId;
    std::string roleVariantId;
    std::string exemplarId;
    std::string lineageAuthority = "LEGACY";
    std::string appearancePresetId;
    bool runtimePcgCertified = true;
    std::string runtimeCertificationMessage;

    std::vector<VisualModulePlacement> modules;
    std::vector<ShipVisualAttachment> attachments;
    std::vector<VisualDetailPlacement> details;
    std::vector<ShipVisualAnchor> anchors;
    std::vector<ShipVisualHardpoint> hardpoints;
};

struct ProceduralVisualCatalog {
    std::uint32_t seed = 1;
    int variantsPerRole = 0;
    std::vector<std::string> sourceModules;
    std::vector<VisualModuleSource> sourceMetrics;
    std::vector<ProceduralShipVisualRecipe> shipRecipes;

    bool Empty() const { return shipRecipes.empty(); }
};

/// Production visual catalog. Pass347+ makes assembly bounds-aware so authored
/// low-poly modules overlap only enough to read as one hull rather than being
/// buried inside each other. Generated recipes remain transforms/references to
/// the repository's existing OBJ module library.
class ProceduralVisualVariantSystem {
public:
    static ProceduralVisualCatalog Build(const std::vector<std::string>& availableModules,
                                         std::uint32_t seed = 0x5A17C0DEu,
                                         int variantsPerRole = 12);

    static ProceduralVisualCatalog Build(const std::vector<VisualModuleSource>& availableModules,
                                         std::uint32_t seed = 0x5A17C0DEu,
                                         int variantsPerRole = 12);

    static const ProceduralShipVisualRecipe* Select(const ProceduralVisualCatalog& catalog,
                                                    const std::string& role,
                                                    std::uint32_t visualSeed);

    static const ProceduralShipVisualRecipe* SelectSourceFamily(const ProceduralVisualCatalog& catalog,
                                                                const std::string& role,
                                                                const std::string& sourceFamily,
                                                                std::uint32_t visualSeed);

    static std::uint32_t StableSeed(const std::string& text);
    static std::string NormalizeRole(const std::string& role);

    /// Pass352 art-director score. Scores below 78 indicate a recipe whose
    /// cockpit/propulsion staging, negative-space readability or functional
    /// module exposure needs to be rejected or repaired before presentation.
    static float EvaluateQuality(const ProceduralShipVisualRecipe& recipe);
};

} // namespace subspace
