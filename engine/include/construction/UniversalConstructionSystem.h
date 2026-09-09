#pragma once

#include "content/UniversalKitbashAuthority.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "station/StationModuleRole.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ConstructionWorkspaceMode {
    Ship,
    Station,
    Planetary,
    Weapon
};

enum class PlanetaryModuleRole {
    Foundation,
    Industry,
    Mining,
    Refinery,
    Power,
    Storage,
    Logistics,
    Habitation,
    Agriculture,
    Research,
    Defense,
    Spaceport,
    Communications,
    Utility,
    Decoration
};

enum class PlanetaryFacilityCapability {
    Foundation,
    Power,
    Production,
    InputStorage,
    OutputStorage,
    LogisticsAccess,
    Communications
};

enum class ConstructionBlueprintDomain {
    Ship,
    Station,
    Planetary,
    Weapon
};

struct ConstructionBlueprintModule {
    std::string assetId;
    std::string derivedVariantId;
    VisualModulePlacement placement{};
    std::string role;
    UniversalSizeClass size = UniversalSizeClass::M;
    std::string sourceAssetId;
};

struct ConstructionBlueprint {
    std::string blueprintId;
    std::string displayName;
    ConstructionBlueprintDomain domain = ConstructionBlueprintDomain::Ship;
    std::string factionId;
    std::string familyId;
    std::string role;
    std::vector<ConstructionBlueprintModule> modules;
    std::vector<VisualAssemblyAttachment> attachments;
    std::string appearancePresetId;
    bool draft = true;
    bool certified = false;
    std::uint32_t version = 1;
};

struct ConstructionBlueprintValidation {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct ManufacturingMaterialRequirement {
    std::string materialId;
    float quantity = 0.0f;
};

struct KitbashManufacturingRecipe {
    std::string recipeId;
    std::string assetId;
    ConstructionDomain domain = ConstructionDomain::Ship;
    UniversalSizeClass size = UniversalSizeClass::M;
    float estimatedMass = 0.0f;
    float manufacturingHours = 0.0f;
    std::vector<ManufacturingMaterialRequirement> materials;
    std::vector<std::string> prerequisiteTechnologies;
};

struct ReverseEngineeringRecord {
    std::string capturedAssetId;
    std::string sourceFactionId;
    std::string sourceBlueprintId;
    float hullKnowledge = 0.0f;
    float moduleKnowledge = 0.0f;
    float materialKnowledge = 0.0f;
    bool manufacturingUnlocked = false;
    std::vector<std::string> learnedAssetIds;
};

class UniversalConstructionSystem {
public:
    // Pass643: shared mode selector; every domain consumes the same asset browser,
    // inspector, drag/drop, blueprint and validation infrastructure.
    static ConstructionDomain Domain(ConstructionWorkspaceMode mode);
    static const char* ModeName(ConstructionWorkspaceMode mode);

    // Pass644-647: domain-specific vocabularies extend universal XS-XL/morph rules.
    static KitbashMorphProfile StationMorphProfile(UniversalSizeClass reference,
                                                   bool structural,
                                                   bool dockingInterface = false);
    static PlanetaryModuleRole InferPlanetaryRole(const UniversalKitbashProfile& profile,
                                                  const std::string& moduleId);
    static std::vector<PlanetaryFacilityCapability> RequiredPlanetaryCapabilities(PlanetaryModuleRole primaryRole);
    static WeaponAssemblyRole NormalizeWeaponRole(const UniversalKitbashProfile& profile);

    // Pass648-649: one blueprint authority for ships/stations/planetary/weapons.
    static ConstructionBlueprint FromShipRecipe(const ProceduralShipVisualRecipe& recipe,
                                                const std::vector<ShipyardModuleRecord>& catalog,
                                                const std::string& blueprintId);
    static ConstructionBlueprintValidation ValidateBlueprint(const ConstructionBlueprint& blueprint);

    // Pass650-651: manufacturing/reverse-engineering are derived from the same
    // canonical identities and retain source/faction provenance.
    static KitbashManufacturingRecipe BuildManufacturingRecipe(const UniversalKitbashProfile& profile,
                                                                ConstructionDomain domain,
                                                                UniversalSizeClass size,
                                                                float sourceVolumeHint = 1.0f);
    static ReverseEngineeringRecord AnalyzeCapturedAsset(const std::string& assetId,
                                                          const std::string& sourceFactionId,
                                                          const std::string& blueprintId,
                                                          float analysisProgress);
};

} // namespace subspace
