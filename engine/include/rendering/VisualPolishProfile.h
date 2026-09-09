#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class VisualPolishLayer {
    Starfield,
    SolarGlow,
    CelestialBodies,
    Asteroids,
    CargoPods,
    MiningBeams,
    ShipThrusters,
    HomeSurface,
    UserInterface
};

struct VisualPolishTuning {
    VisualPolishLayer layer = VisualPolishLayer::Starfield;
    std::string label;
    float intensity = 1.0f;
    float contrast = 1.0f;
    float detail = 1.0f;
    bool enabled = true;
};

struct VisualPolishProfile {
    std::string profileId = "subspace.default";
    std::string displayName = "Subspace Dark Industrial";
    std::uint32_t accentColor = 0x66D9FFu;
    std::uint32_t warningColor = 0xFFB347u;
    std::uint32_t hazardColor = 0xFF5577u;
    std::vector<VisualPolishTuning> layers;
};

const char* VisualPolishLayerName(VisualPolishLayer layer);
VisualPolishProfile CreateDefaultVisualPolishProfile();
VisualPolishProfile CreateHighReadabilityVisualPolishProfile();
float ResolveLayerIntensity(const VisualPolishProfile& profile, VisualPolishLayer layer);
std::vector<std::string> BuildVisualPolishChecklist(const VisualPolishProfile& profile);

} // namespace subspace
