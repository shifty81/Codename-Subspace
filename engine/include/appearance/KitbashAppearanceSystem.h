#pragma once

#include "assets/CanonicalAsset.h"
#include "ships/ShipSpatialAssemblySystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class AppearanceChannel {
    Primary,
    Secondary,
    Accent,
    Structural,
    Functional,
    Emissive,
    Unpainted
};

enum class PatternProjectionMode {
    SurfaceUv,
    ModuleLocal,
    ShipSpace,
    Triplanar,
    DirectionalStripe
};

struct AppearanceRegionRule {
    ShipSpatialRegion region = ShipSpatialRegion::MidHull;
    assets::SurfaceSemantic surface = assets::SurfaceSemantic::Unknown;
    AppearanceChannel channel = AppearanceChannel::Primary;
    bool patternEligible = true;
    bool decalEligible = true;
    bool weatheringEligible = true;
};

struct AppearancePresetDefinition {
    std::string id;
    std::string parentId;
    std::vector<AppearanceRegionRule> rules;
    PatternProjectionMode hullPatternProjection = PatternProjectionMode::ShipSpace;
    PatternProjectionMode localWarningProjection = PatternProjectionMode::ModuleLocal;
};

class KitbashAppearanceSystem {
public:
    static std::vector<AppearanceRegionRule> DefaultRules();
    static AppearancePresetDefinition BuildFactionPreset(const std::string& factionId,
                                                          const std::string& variant = "DEFAULT");
    static AppearanceChannel Resolve(const AppearancePresetDefinition& preset,
                                     ShipSpatialRegion region,
                                     assets::SurfaceSemantic surface);
    static bool MirrorsDecalGlyphs(bool decalContainsReadableText);
};

} // namespace subspace
