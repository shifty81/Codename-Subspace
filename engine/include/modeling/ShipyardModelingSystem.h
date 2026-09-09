#pragma once

#include "assets/CanonicalAsset.h"
#include "core/Math.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ModelingPrimitiveType {
    Box,
    Wedge,
    Cylinder,
    Cone,
    Sphere,
    Tube,
    Ring,
    Beam,
    Plate,
    HullSegment,
    Wing,
    EngineHousing,
    Nozzle,
    TurretRing,
    Barrel,
    Pipe
};

enum class ModelingSelectionMode { Object, Vertex, Edge, Face };

enum class ModelingModifierType {
    Transform,
    Stretch,
    Taper,
    Bend,
    Twist,
    Bevel,
    Inset,
    Extrude,
    Mirror,
    LinearArray,
    RadialArray,
    BooleanUnion,
    BooleanSubtract,
    BooleanIntersect
};

struct ModelingPrimitiveDefinition {
    std::string id;
    ModelingPrimitiveType type = ModelingPrimitiveType::Box;
    Vector3 size{1.0f, 1.0f, 1.0f};
    Vector3 position{};
    Vector3 rotationDegrees{};
    std::uint32_t radialSegments = 16;
    float bevel = 0.0f;
    float wallThickness = 0.08f;
    std::string surfaceSemantic = "HullPrimary";
};

struct ModelingModifier {
    std::string id;
    ModelingModifierType type = ModelingModifierType::Transform;
    Vector3 vector{};
    float amount = 0.0f;
    std::uint32_t count = 1;
    bool enabled = true;
    bool preserveSockets = true;
    bool preserveFunctionalRegions = true;
};

struct ShipyardModelRecipe {
    std::string recipeId = "shipyard.model.untitled";
    std::string displayName = "Untitled Module";
    std::string sourceAssetId;
    ModelingSelectionMode selectionMode = ModelingSelectionMode::Object;
    std::vector<ModelingPrimitiveDefinition> primitives;
    std::vector<ModelingModifier> modifiers;
    bool draft = true;
    bool collisionDirty = true;
    bool socketsDirty = false;
    bool surfacesDirty = false;
    std::uint32_t revision = 1;
};

struct ShipyardModelingValidation {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct ShipyardModelingState {
    ShipyardModelRecipe recipe{};
    ModelingPrimitiveType selectedPrimitive = ModelingPrimitiveType::Box;
    ModelingSelectionMode selectionMode = ModelingSelectionMode::Object;
    std::size_t selectedPrimitiveIndex = 0;
    float stretchStep = 0.10f;
    bool symmetricStretch = false;
    bool autoCollision = true;
    bool liveCanonicalPreview = true;
    std::string status = "Model workspace ready";
};

class ShipyardModelingSystem {
public:
    static const char* PrimitiveName(ModelingPrimitiveType type);
    static const char* SelectionModeName(ModelingSelectionMode mode);
    static const char* ModifierName(ModelingModifierType type);

    static ModelingPrimitiveDefinition DefaultPrimitive(ModelingPrimitiveType type,
                                                        std::size_t ordinal = 0);
    static std::size_t AddPrimitive(ShipyardModelRecipe& recipe,
                                    ModelingPrimitiveType type);
    static bool StretchPrimitive(ShipyardModelRecipe& recipe,
                                 std::size_t index,
                                 const Vector3& delta,
                                 bool symmetric);
    static bool AddModifier(ShipyardModelRecipe& recipe,
                            ModelingModifier modifier);
    static ShipyardModelingValidation Validate(const ShipyardModelRecipe& recipe);

    // Generates real canonical geometry for the primitive/model recipe. The
    // source recipe remains authoritative and can be re-baked non-destructively.
    static assets::CanonicalAsset BakeCanonicalAsset(const ShipyardModelRecipe& recipe,
                                                      const std::string& assetId);
    static std::string DerivedAssetId(const ShipyardModelRecipe& recipe,
                                      const std::string& requestedAssetId = {});
};

} // namespace subspace
