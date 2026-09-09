#pragma once

#include "procedural/GalaxyGenerator.h"
#include "navigation/VectorTravelSystem.h"

#include <string>

namespace subspace {

struct SpaceBackdropProfile {
    int ultraDistantStars = 180;
    int distantStars = 260;
    int localStars = 120;
    float galacticBandStrength = 0.12f;
    float nebulaHaze = 0.035f;
    float localDust = 0.04f;
    float exposure = 1.0f;
    float warmBias = 0.0f;
    float nebulaR = 0.10f;
    float nebulaG = 0.16f;
    float nebulaB = 0.28f;
    float dustLaneStrength = 0.10f;
    float starTwinkle = 0.12f;
    float galacticBandTilt = 0.0f;
    std::string identity;
};


struct ShipPresentationProfile {
    float lengthScale = 1.0f;
    float widthScale = 1.0f;
    float platingBreakup = 0.65f;
    float machineryExposure = 0.35f;
    float thrusterGlow = 1.0f;
    float navigationLights = 0.8f;
    float hullCohesion = 0.72f;
    float materialRoughness = 0.62f;
    float metallicResponse = 0.68f;
    float emissiveAccent = 0.34f;
    float retroThrusterIntensity = 0.82f;
    float maneuverThrusterIntensity = 0.70f;
    float damageScorch = 0.0f;
    float lodDetailScale = 1.0f;
    int lod = 0;
    std::string identity = "UTILITY";
};

struct VectorVisualProfile {
    float distortion = 0.0f;
    float starStretch = 0.0f;
    float tunnelOpacity = 0.0f;
    float cameraPullback = 0.0f;
    float exitReveal = 0.0f;
    float entryFlash = 0.0f;
    float chromaticShift = 0.0f;
    float tunnelFlow = 0.0f;
    float audioIntensity = 0.0f;
    float destinationReveal = 0.0f;
    float tunnelTwist = 0.0f;
    float tunnelPulse = 0.0f;
    float shipEnvelope = 0.0f;
    float chaseBias = 0.0f;
    float engineOverdrive = 0.0f;
    float foregroundStreaks = 0.0f;
};

/// Pass296-300/303 presentation authority. It produces deterministic visual
/// profiles; the renderer consumes the profile but never owns simulation state.
class ForwardSpacePresentationSystem {
public:
    SpaceBackdropProfile ForSector(const GalaxySector& sector) const;
    ShipPresentationProfile ForShip(const std::string& role, bool player, float screenFraction) const;
    VectorVisualProfile VectorVisual(VectorTravelStage stage, double stageProgress) const;
};

} // namespace subspace
