#pragma once

#include "core/Math.h"

namespace subspace {

enum class RenderingMode { PBR, NPR, Hybrid };
enum class RenderingPreset { RealisticPBR, StylizedNPR, HybridBalanced, Performance };

/// Native replacement for the legacy C# RenderingConfiguration singleton.
/// Kept as plain engine data so the renderer/backend can own lifetime and
/// persistence without a hidden global .NET-style singleton.
struct RenderingConfiguration {
    RenderingMode mode = RenderingMode::Hybrid;

    bool enableEdgeDetection = true;
    float edgeThickness = 1.2f;
    Vector3 edgeColor{0.10f, 0.10f, 0.15f};
    bool enableCelShading = false;
    int celShadingBands = 4;

    bool enableAmbientOcclusion = true;
    float ambientOcclusionStrength = 0.35f;
    bool enablePerMaterialProperties = true;
    bool enableProceduralDetails = true;
    float proceduralDetailStrength = 0.50f;

    bool enableBlockGlow = true;
    float blockGlowIntensity = 1.0f;
    bool enableBlockTypeColoring = true;

    bool enableRimLighting = true;
    float rimLightingStrength = 0.40f;
    bool enableEnvironmentReflections = true;

    void ApplyPreset(RenderingPreset preset);
    bool Validate() const;
};

} // namespace subspace
