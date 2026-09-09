#include "rendering/PlanetSurfaceSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {

float SeedUnit(int seed, int salt) {
    std::uint32_t x = static_cast<std::uint32_t>(seed) ^ (0x9E3779B9u * static_cast<std::uint32_t>(salt + 1));
    x ^= x >> 16; x *= 0x7FEB352Du; x ^= x >> 15; x *= 0x846CA68Bu; x ^= x >> 16;
    return static_cast<float>(x & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
}

std::array<float,3> Jitter(std::array<float,3> color, float amount, float seed) {
    const float delta = (seed - 0.5f) * amount;
    for (float& c : color) c = std::clamp(c + delta, 0.02f, 1.0f);
    return color;
}

} // namespace

PlanetSurfaceProfile PlanetSurfaceSystem::Build(const PlanetData& planet) {
    PlanetSurfaceProfile p;
    const float a = SeedUnit(planet.surfaceSeed, 2);
    const float b = SeedUnit(planet.surfaceSeed, 7);
    const float c = SeedUnit(planet.surfaceSeed, 13);
    p.surfaceSeed = 1.0f + static_cast<float>(std::abs(planet.surfaceSeed % 100000)) * 0.001f;

    switch (planet.type) {
        case PlanetType::Rocky:
            p.material = SpaceMaterialKind::PlanetRock;
            p.baseColor = Jitter({0.43f,0.39f,0.34f},0.10f,a);
            p.detailColor = Jitter({0.20f,0.19f,0.18f},0.08f,b);
            p.atmosphereColor = {0.42f,0.50f,0.58f};
            p.atmosphereOpacity = 0.07f + 0.04f*c;
            p.surfaceVariation = 0.62f; p.detailScale = 10.0f + 5.0f*b;
            p.descriptor = "cratered rock / exposed mineral terrain";
            break;
        case PlanetType::Desert:
            p.material = SpaceMaterialKind::PlanetDesert;
            p.baseColor = Jitter({0.68f,0.38f,0.16f},0.12f,a);
            p.detailColor = Jitter({0.91f,0.67f,0.31f},0.10f,b);
            p.atmosphereColor = {0.83f,0.53f,0.27f};
            p.cloudColor = {0.86f,0.70f,0.50f};
            p.atmosphereOpacity = 0.10f + 0.06f*c; p.cloudOpacity = 0.018f;
            p.surfaceVariation = 0.52f; p.detailScale = 8.0f + 4.0f*b;
            p.bandStrength = 0.10f + 0.14f*c;
            p.descriptor = "dune fields / dry basins / exposed ridges";
            break;
        case PlanetType::Ice:
            p.material = SpaceMaterialKind::PlanetIce;
            p.baseColor = Jitter({0.58f,0.78f,0.89f},0.08f,a);
            p.detailColor = {0.86f,0.94f,0.98f};
            p.atmosphereColor = {0.55f,0.78f,0.94f};
            p.cloudColor = {0.94f,0.98f,1.0f};
            p.atmosphereOpacity = 0.10f; p.cloudOpacity = 0.035f;
            p.surfaceVariation = 0.48f; p.detailScale = 13.0f;
            p.iceFraction = 0.75f + 0.22f*b;
            p.descriptor = "ice shelves / fractured glaciers / frozen plains";
            break;
        case PlanetType::Oceanic:
            p.material = SpaceMaterialKind::PlanetOcean;
            p.baseColor = Jitter({0.07f,0.28f,0.58f},0.06f,a);
            p.detailColor = Jitter({0.23f,0.48f,0.24f},0.10f,b);
            p.atmosphereColor = {0.20f,0.62f,0.91f};
            p.cloudColor = {0.93f,0.97f,1.0f};
            p.atmosphereOpacity = 0.16f; p.cloudOpacity = 0.065f;
            p.surfaceVariation = 0.70f; p.detailScale = 6.5f;
            p.oceanFraction = 0.56f + 0.30f*a;
            p.stormStrength = 0.12f + 0.42f*planet.hazardLevel;
            p.descriptor = "deep ocean / island chains / cloud systems";
            break;
        case PlanetType::Volcanic:
            p.material = SpaceMaterialKind::PlanetVolcanic;
            p.baseColor = Jitter({0.20f,0.07f,0.045f},0.05f,a);
            p.detailColor = {0.72f,0.18f,0.035f};
            p.atmosphereColor = {0.58f,0.18f,0.07f};
            p.cloudColor = {0.30f,0.22f,0.20f};
            p.atmosphereOpacity = 0.13f; p.cloudOpacity = 0.045f;
            p.surfaceVariation = 0.86f; p.detailScale = 16.0f;
            p.lavaGlow = 0.55f + 0.40f*planet.hazardLevel;
            p.descriptor = "basalt crust / lava rifts / volcanic calderas";
            break;
        case PlanetType::Barren:
            p.material = SpaceMaterialKind::PlanetBarren;
            p.baseColor = Jitter({0.34f,0.29f,0.25f},0.07f,a);
            p.detailColor = Jitter({0.47f,0.42f,0.36f},0.05f,b);
            p.atmosphereColor = {0.34f,0.38f,0.42f};
            p.atmosphereOpacity = 0.035f;
            p.surfaceVariation = 0.72f; p.detailScale = 18.0f;
            p.descriptor = "airless regolith / impact basins / fractured highlands";
            break;
        case PlanetType::GasGiant:
            p.material = SpaceMaterialKind::PlanetGas;
            p.baseColor = Jitter({0.57f,0.43f,0.29f},0.13f,a);
            p.detailColor = Jitter({0.87f,0.72f,0.50f},0.12f,b);
            p.atmosphereColor = Jitter({0.68f,0.52f,0.38f},0.10f,c);
            p.cloudColor = {0.91f,0.84f,0.72f};
            p.atmosphereOpacity = 0.19f; p.cloudOpacity = 0.075f;
            p.surfaceVariation = 0.76f; p.detailScale = 7.0f;
            p.bandStrength = 0.82f + 0.15f*b;
            p.stormStrength = 0.30f + 0.62f*planet.hazardLevel;
            p.descriptor = "layered atmosphere / jet bands / persistent storms";
            break;
    }

    // Rich/resource-heavy worlds get slightly more material contrast so the
    // visual surface hints at their geology without exposing exact survey data.
    p.surfaceVariation = std::clamp(p.surfaceVariation + planet.resourceRichness * 0.08f, 0.0f, 1.0f);
    return p;
}

const char* PlanetSurfaceSystem::IndustryRepresentation(const PlanetData& planet) {
    return planet.type == PlanetType::GasGiant ? "Atmospheric Collector Ring" : "Surface Hex Industry";
}

} // namespace subspace
