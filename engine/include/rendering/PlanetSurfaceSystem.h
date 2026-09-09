#pragma once

#include "procedural/GalaxyGenerator.h"
#include "rendering/SpaceMaterialSystem.h"

#include <array>
#include <string>

namespace subspace {

/// Data-driven planet appearance derived from the same seed/type/richness data
/// used by gameplay. No planet is represented by a single flat color anymore;
/// the strategic renderer receives a stable surface/atmosphere profile that is
/// regenerated identically on every load.
struct PlanetSurfaceProfile {
    SpaceMaterialKind material = SpaceMaterialKind::PlanetRock;
    std::array<float,3> baseColor{0.45f,0.42f,0.38f};
    std::array<float,3> detailColor{0.25f,0.23f,0.21f};
    std::array<float,3> atmosphereColor{0.42f,0.56f,0.68f};
    std::array<float,3> cloudColor{0.88f,0.91f,0.94f};
    float atmosphereOpacity = 0.10f;
    float cloudOpacity = 0.04f;
    float surfaceVariation = 0.45f;
    float detailScale = 7.0f;
    float bandStrength = 0.0f;
    float lavaGlow = 0.0f;
    float oceanFraction = 0.0f;
    float iceFraction = 0.0f;
    float stormStrength = 0.0f;
    float surfaceSeed = 1.0f;
    std::string descriptor;
};

class PlanetSurfaceSystem {
public:
    static PlanetSurfaceProfile Build(const PlanetData& planet);
    static const char* IndustryRepresentation(const PlanetData& planet);
};

} // namespace subspace
