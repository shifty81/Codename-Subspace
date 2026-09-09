#include "rendering/VisualPolishProfile.h"

#include <algorithm>

namespace subspace {

const char* VisualPolishLayerName(VisualPolishLayer layer) {
    switch (layer) {
    case VisualPolishLayer::Starfield: return "Starfield";
    case VisualPolishLayer::SolarGlow: return "SolarGlow";
    case VisualPolishLayer::CelestialBodies: return "CelestialBodies";
    case VisualPolishLayer::Asteroids: return "Asteroids";
    case VisualPolishLayer::CargoPods: return "CargoPods";
    case VisualPolishLayer::MiningBeams: return "MiningBeams";
    case VisualPolishLayer::ShipThrusters: return "ShipThrusters";
    case VisualPolishLayer::HomeSurface: return "HomeSurface";
    case VisualPolishLayer::UserInterface: return "UserInterface";
    }
    return "Unknown";
}

static VisualPolishTuning MakeLayer(VisualPolishLayer layer, float intensity, float contrast, float detail) {
    VisualPolishTuning tuning;
    tuning.layer = layer;
    tuning.label = VisualPolishLayerName(layer);
    tuning.intensity = intensity;
    tuning.contrast = contrast;
    tuning.detail = detail;
    tuning.enabled = true;
    return tuning;
}

VisualPolishProfile CreateDefaultVisualPolishProfile() {
    VisualPolishProfile profile;
    profile.layers = {
        MakeLayer(VisualPolishLayer::Starfield, 0.80f, 0.85f, 0.75f),
        MakeLayer(VisualPolishLayer::SolarGlow, 1.10f, 1.00f, 0.90f),
        MakeLayer(VisualPolishLayer::CelestialBodies, 1.00f, 1.10f, 1.00f),
        MakeLayer(VisualPolishLayer::Asteroids, 1.00f, 1.05f, 1.15f),
        MakeLayer(VisualPolishLayer::CargoPods, 1.00f, 1.30f, 0.85f),
        MakeLayer(VisualPolishLayer::MiningBeams, 1.20f, 1.25f, 0.90f),
        MakeLayer(VisualPolishLayer::ShipThrusters, 1.30f, 1.20f, 1.00f),
        MakeLayer(VisualPolishLayer::HomeSurface, 1.00f, 1.10f, 1.00f),
        MakeLayer(VisualPolishLayer::UserInterface, 0.95f, 1.35f, 0.80f),
    };
    return profile;
}

VisualPolishProfile CreateHighReadabilityVisualPolishProfile() {
    auto profile = CreateDefaultVisualPolishProfile();
    profile.profileId = "subspace.high_readability";
    profile.displayName = "Subspace High Readability";
    for (auto& layer : profile.layers) {
        layer.contrast = std::max(layer.contrast, 1.25f);
        layer.intensity = std::max(layer.intensity, 1.05f);
    }
    return profile;
}

float ResolveLayerIntensity(const VisualPolishProfile& profile, VisualPolishLayer layer) {
    for (const auto& entry : profile.layers) {
        if (entry.layer == layer) {
            return entry.enabled ? std::max(0.0f, entry.intensity) : 0.0f;
        }
    }
    return 1.0f;
}

std::vector<std::string> BuildVisualPolishChecklist(const VisualPolishProfile& profile) {
    std::vector<std::string> checklist;
    checklist.push_back("Profile: " + profile.displayName);
    for (const auto& layer : profile.layers) {
        checklist.push_back(std::string(layer.enabled ? "PASS " : "OFF  ") + layer.label);
    }
    return checklist;
}

} // namespace subspace
