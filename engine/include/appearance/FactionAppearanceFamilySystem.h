#pragma once

#include "appearance/KitbashAppearanceSystem.h"
#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipAppearanceZone {
    Global,
    Bow,
    Command,
    ForwardHull,
    MidHull,
    AftHull,
    PortWing,
    StarboardWing,
    Dorsal,
    Ventral,
    MainPropulsion,
    Maneuvering,
    VtolLanding,
    Weapons,
    Sensors,
    Docking,
    Industrial,
    Structural
};

struct SurfaceRegionBinding {
    std::size_t moduleIndex = 0;
    std::string moduleId;
    ShipAppearanceZone zone = ShipAppearanceZone::Global;
    assets::SurfaceSemantic semantic = assets::SurfaceSemantic::Unknown;
    AppearanceChannel channel = AppearanceChannel::Primary;
    bool patternEligible = true;
    bool decalEligible = true;
    bool weatheringEligible = true;
};

struct FactionAppearanceFamily {
    std::string factionId;
    std::string basePresetId = "SUBSPACE_DEFAULT";
    std::string shipPresetId;
    std::string stationPresetId;
    std::string planetaryPresetId;
    std::string weaponPresetId;
    AppearancePresetDefinition shipPreset{};
};

class FactionAppearanceFamilySystem {
public:
    // Pass639-642: one semantic faction livery family spans construction domains.
    static FactionAppearanceFamily Build(const std::string& factionId,
                                         const std::string& variant = "DEFAULT");
    static ShipAppearanceZone ZoneFor(const ShipyardModuleRecord& record,
                                      const VisualModulePlacement& placement,
                                      const ProceduralShipVisualRecipe& recipe);
    static std::vector<SurfaceRegionBinding> SegmentShip(const std::vector<ShipyardModuleRecord>& catalog,
                                                         const ProceduralShipVisualRecipe& recipe,
                                                         const AppearancePresetDefinition& preset);
    static AppearancePresetDefinition Inherit(const AppearancePresetDefinition& parent,
                                              const AppearancePresetDefinition& child);
    static PatternProjectionMode ProjectionFor(ShipAppearanceZone zone,
                                               bool readableDecalText = false);
};

} // namespace subspace
