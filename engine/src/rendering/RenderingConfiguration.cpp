#include "rendering/RenderingConfiguration.h"

namespace subspace {

void RenderingConfiguration::ApplyPreset(RenderingPreset preset)
{
    switch (preset) {
        case RenderingPreset::RealisticPBR:
            mode = RenderingMode::PBR;
            enableEdgeDetection = false;
            enableCelShading = false;
            enableAmbientOcclusion = true;
            ambientOcclusionStrength = 0.40f;
            enablePerMaterialProperties = true;
            enableProceduralDetails = true;
            proceduralDetailStrength = 0.60f;
            enableBlockGlow = true;
            blockGlowIntensity = 0.80f;
            enableRimLighting = true;
            rimLightingStrength = 0.30f;
            enableEnvironmentReflections = true;
            break;

        case RenderingPreset::StylizedNPR:
            mode = RenderingMode::NPR;
            enableEdgeDetection = true;
            edgeThickness = 1.50f;
            enableCelShading = true;
            celShadingBands = 4;
            enableAmbientOcclusion = false;
            enablePerMaterialProperties = true;
            enableProceduralDetails = false;
            enableBlockGlow = true;
            blockGlowIntensity = 1.20f;
            enableRimLighting = true;
            rimLightingStrength = 0.50f;
            enableEnvironmentReflections = false;
            break;

        case RenderingPreset::HybridBalanced:
            mode = RenderingMode::Hybrid;
            enableEdgeDetection = true;
            edgeThickness = 1.00f;
            edgeColor = {0.15f, 0.15f, 0.20f};
            enableCelShading = false;
            enableAmbientOcclusion = true;
            ambientOcclusionStrength = 0.35f;
            enablePerMaterialProperties = true;
            enableProceduralDetails = true;
            proceduralDetailStrength = 0.40f;
            enableBlockGlow = true;
            blockGlowIntensity = 1.0f;
            enableRimLighting = true;
            rimLightingStrength = 0.40f;
            enableEnvironmentReflections = true;
            break;

        case RenderingPreset::Performance:
            mode = RenderingMode::PBR;
            enableEdgeDetection = false;
            enableCelShading = false;
            enableAmbientOcclusion = false;
            enablePerMaterialProperties = false;
            enableProceduralDetails = false;
            enableBlockGlow = false;
            enableRimLighting = false;
            enableEnvironmentReflections = false;
            break;
    }
}

bool RenderingConfiguration::Validate() const
{
    return edgeThickness > 0.0f &&
           celShadingBands >= 2 && celShadingBands <= 16 &&
           ambientOcclusionStrength >= 0.0f && ambientOcclusionStrength <= 1.0f &&
           proceduralDetailStrength >= 0.0f && proceduralDetailStrength <= 1.0f &&
           blockGlowIntensity >= 0.0f && blockGlowIntensity <= 4.0f &&
           rimLightingStrength >= 0.0f && rimLightingStrength <= 2.0f;
}

} // namespace subspace
