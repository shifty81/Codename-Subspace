#pragma once

#include <string>

namespace subspace {

enum class SpaceMaterialKind {
    ShipHull,
    IndustrialHull,
    Canopy,
    EngineHousing,
    ThrusterCore,
    ArmorPlate,
    StructuralMetal,
    Radiator,
    HeatShield,
    DecalSurface,
    AsteroidRock,
    OreVein,
    StationHull,
    PlanetRock,
    PlanetDesert,
    PlanetOcean,
    PlanetIce,
    PlanetVolcanic,
    PlanetBarren,
    PlanetGas,
    Sun,
    MissileBody,
    MissileExhaust,
    Dust,
    Debris
};

struct SpaceMaterialProfile {
    SpaceMaterialKind kind = SpaceMaterialKind::ShipHull;
    float roughness = 0.55f;
    float metallic = 0.55f;
    float emissive = 0.0f;
    float rimStrength = 0.14f;
    float celBands = 4.0f;
    float specularStrength = 0.35f;
    float fresnelStrength = 0.12f;
    float edgeHighlight = 0.0f;
    float cavityStrength = 0.0f;
    float wearStrength = 0.0f;

    // Pass338-340 shader-driven surface authority. surfaceMode 0 is ordinary
    // ship/station/debris material; 1..7 are procedural planetary surfaces.
    float surfaceMode = 0.0f;
    float surfaceVariation = 0.0f;
    float detailScale = 4.0f;
    float bandStrength = 0.0f;
    float lavaGlow = 0.0f;
    float detailR = 0.40f;
    float detailG = 0.42f;
    float detailB = 0.44f;
};

/// Native material/shader contract for the strategic renderer. The gameplay
/// simulation has no dependency on these values; they exist strictly to keep
/// ships, stations, celestials, missiles and debris visually coherent.
class SpaceMaterialSystem {
public:
    static SpaceMaterialProfile GetProfile(SpaceMaterialKind kind);

    /// Compatibility-profile GLSL used by the Win32/WGL renderer. Pass338
    /// keeps GLSL 1.20 compatibility while adding Fresnel response, smoother
    /// energy-conserving highlights and deterministic shader-driven planetary
    /// terrain/bands. Fixed-function remains a fail-safe only.
    static const char* VertexShader120();
    static const char* FragmentShader120();
};

} // namespace subspace
