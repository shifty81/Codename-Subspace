#pragma once

namespace subspace::SystemSpatialScale {
// One macro/procedural sector unit maps to this many native flight-world units.
// Keeping this value shared prevents procedural placement safety from drifting
// away from the renderer's celestial presentation scale.
inline constexpr float SectorToWorld = 0.0120f;
inline constexpr float WorldToSector = 1.0f / SectorToWorld;
}
