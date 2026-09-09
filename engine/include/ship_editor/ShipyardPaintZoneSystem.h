#pragma once

#include <string_view>

namespace subspace {

enum class ShipyardPaintZone {
    Primary,
    Secondary,
    Trim,
    StructuralDark,
    LightMetal,
    DarkMetal,
    EmissiveWhite,
    EmissiveBlue,
    EmissiveOrange,
    Glass,
    Inherited
};

/// Maps Greyoxide/source MTL material regions into stable Subspace livery
/// zones.  This keeps one physical mesh while allowing genuinely different
/// colors/material treatment across its authored face groups.
class ShipyardPaintZoneSystem {
public:
    static ShipyardPaintZone ForMaterial(std::string_view materialName);
    static const char* Name(ShipyardPaintZone zone);
    static bool IsPaintable(ShipyardPaintZone zone);
};

} // namespace subspace
