#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "ships/ShipClassRoleSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct ShipGenerationBounds {
    bool valid = false;
    Vector3 minimum{};
    Vector3 maximum{};
    Vector3 size{};
};

struct ShipClassGenerationReport {
    bool valid = false;
    ShipClass shipClass = ShipClass::Frigate;
    UniversalSizeClass requestedSize = UniversalSizeClass::S;
    UniversalSizeClass resolvedSize = UniversalSizeClass::S;
    float measuredLengthMeters = 0.0f;
    float measuredWidthMeters = 0.0f;
    float measuredHeightMeters = 0.0f;
    float targetLengthMeters = 0.0f;
    float appliedScale = 1.0f;
    float maximumInstanceScale = 1.0f;
    bool topologyRegenerationRequired = false;
    std::size_t moduleCount = 0;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Hard generation authority for ship class + XS-XL. The selected class owns
/// the legal module-size family and physical hull envelope. XS-XL chooses a
/// target inside that envelope, but topology/module-family selection must do the
/// real size work. Only a small final metric correction is permitted.
class ShipClassGenerationAuthoritySystem {
public:
    // Generation may make only a small final metric correction. Class size must
    // be realized primarily by topology/module-family selection, never by
    // inflating an entire small craft into a capital hull.
    static constexpr float MinimumFinalCorrectionScale = 0.85f;
    static constexpr float MaximumFinalCorrectionScale = 1.15f;
    static constexpr float MaximumGeneratedInstanceScale = 1.50f;

    static ShipGenerationBounds MeasureBoundsMeters(const ProceduralShipVisualRecipe& recipe,
                                                     const std::vector<ShipyardModuleRecord>& catalog);
    static float MeasureLengthMeters(const ProceduralShipVisualRecipe& recipe,
                                     const std::vector<ShipyardModuleRecord>& catalog);
    static float TargetLengthMeters(ShipClass shipClass,
                                    UniversalSizeClass requestedSize,
                                    UniversalSizeClass* resolvedSize = nullptr);
    static ShipClassGenerationReport Resolve(const ProceduralShipVisualRecipe& recipe,
                                             const std::vector<ShipyardModuleRecord>& catalog,
                                             ShipClass shipClass,
                                             UniversalSizeClass requestedSize);
    static float CombinedScale(const ProceduralShipVisualRecipe& recipe,
                               const std::vector<ShipyardModuleRecord>& catalog,
                               ShipClass shipClass,
                               UniversalSizeClass requestedSize,
                               UniversalSizeClass* resolvedSize = nullptr);
    static void ApplyScale(ProceduralShipVisualRecipe& recipe, float uniformScale);
    static ShipClassGenerationReport ApplyAndStamp(ProceduralShipVisualRecipe& recipe,
                                                   const std::vector<ShipyardModuleRecord>& catalog,
                                                   ShipClass shipClass,
                                                   UniversalSizeClass requestedSize);
    static bool Validate(const ProceduralShipVisualRecipe& recipe,
                         const std::vector<ShipyardModuleRecord>& catalog,
                         ShipClass shipClass,
                         UniversalSizeClass requestedSize,
                         std::string* error = nullptr);
};

} // namespace subspace
