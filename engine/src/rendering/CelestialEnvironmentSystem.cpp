#include "rendering/CelestialEnvironmentSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

CelestialPresentationProfile CelestialEnvironmentSystem::ProfileFor(const PlanetData& planet) const {
    CelestialPresentationProfile p;
    p.radiusMultiplier = 4.80f;
    p.atmosphereShellMultiplier = 1.045f;
    p.hasCloudLayer = planet.type == PlanetType::Oceanic || planet.type == PlanetType::GasGiant || planet.type == PlanetType::Desert;
    if (planet.type == PlanetType::GasGiant) {
        p.radiusMultiplier = 6.80f;
        p.ringOuterMultiplier = 4.85f;
        p.localDominanceRadiusMultiplier = 4.8f;
        p.maximumScreenFraction = 4.50f;
    } else if (planet.type == PlanetType::Ice) {
        p.radiusMultiplier = 5.20f;
        p.atmosphereShellMultiplier = 1.045f;
    } else if (planet.type == PlanetType::Volcanic) {
        p.radiusMultiplier = 5.00f;
        p.atmosphereShellMultiplier = 1.035f;
    }
    p.supportsDenseRingField = planet.hasRings || planet.type == PlanetType::GasGiant;
    p.surfaceDetail = planet.type == PlanetType::GasGiant ? .82f : .72f;
    p.cloudOpacity = p.hasCloudLayer ? (planet.type == PlanetType::GasGiant ? .34f : .28f) : 0.0f;
    p.atmosphereGlow = planet.type == PlanetType::Volcanic ? .11f : .17f;
    p.ringShadowStrength = p.supportsDenseRingField ? .58f : 0.0f;
    p.nightLightStrength = planet.elevatorCandidate ? .24f : 0.0f;
    return p;
}

float CelestialEnvironmentSystem::WorldRadius(const PlanetData& planet) const {
    const auto p = ProfileFor(planet);
    // Flight-space celestial scale is intentionally much larger than map
    // icon scale. At orbital arrival a terrestrial world should dominate the
    // composition and a gas giant can extend well beyond the viewport.
    const float base = std::clamp(planet.radius * 0.060f, 14.0f, 96.0f);
    const float maxRadius = planet.type == PlanetType::GasGiant ? 720.0f : 420.0f;
    return std::clamp(base * p.radiusMultiplier, 68.0f, maxRadius);
}

float CelestialEnvironmentSystem::MinimumOrbitalSeparation(const PlanetData& a, const PlanetData& b) const {
    const float largest = std::max(a.radius, b.radius);
    return std::max(150000.0f, largest * 180.0f);
}

float CelestialEnvironmentSystem::SafeLocalOrbitRadius(const PlanetData& planet, float additionalClearance) const {
    const auto profile = ProfileFor(planet);
    const float body = WorldRadius(planet);
    const float shell = profile.supportsDenseRingField
        ? body * std::min(5.4f, profile.ringOuterMultiplier + 0.35f)
        // Pass429: non-ringed orbital neighborhoods should read as orbit, not
        // deep space. 1.10x remains safely outside the atmosphere shell while
        // keeping the planet visible from stations/arrival points.
        : body * 1.10f;
    return shell + std::max(0.0f, additionalClearance);
}

Vector3 CelestialEnvironmentSystem::ProjectOutsideLocalOrbit(
    const Vector3& candidate, const Vector3& planetWorld, const PlanetData& planet, float additionalClearance) const {
    const float safe = SafeLocalOrbitRadius(planet, additionalClearance);
    float dx = candidate.x - planetWorld.x, dy = candidate.y - planetWorld.y;
    float d = std::sqrt(dx*dx + dy*dy);
    if (d >= safe) return candidate;
    if (d < 0.001f) { dx = 1.0f; dy = 0.0f; d = 1.0f; }
    return {planetWorld.x + dx/d*safe, planetWorld.y + dy/d*safe, candidate.z};
}

std::vector<RingPresentationBand> CelestialEnvironmentSystem::RingBands(const PlanetData& planet) const {
    if (!planet.hasRings && planet.type != PlanetType::GasGiant) return {};
    const auto p = ProfileFor(planet);
    const float span = p.ringOuterMultiplier - p.ringInnerMultiplier;
    std::vector<RingPresentationBand> bands;
    bands.push_back({p.ringInnerMultiplier, p.ringInnerMultiplier + span * 0.15f, 0.22f, 0.45f});
    bands.push_back({p.ringInnerMultiplier + span * 0.20f, p.ringInnerMultiplier + span * 0.46f, 0.58f, 0.88f});
    bands.push_back({p.ringInnerMultiplier + span * 0.50f, p.ringInnerMultiplier + span * 0.66f, 0.31f, 0.62f});
    bands.push_back({p.ringInnerMultiplier + span * 0.72f, p.ringOuterMultiplier, 0.46f, 0.78f});
    return bands;
}

CelestialLocalContext CelestialEnvironmentSystem::EvaluateLocalContext(const Vector3& playerWorld,
                                                                        const Vector3& planetWorld,
                                                                        const PlanetData& planet,
                                                                        bool nearestMajor) const {
    CelestialLocalContext c;
    const float dx = playerWorld.x - planetWorld.x;
    const float dy = playerWorld.y - planetWorld.y;
    c.distanceWorld = std::sqrt(dx * dx + dy * dy);
    c.radiusWorld = WorldRadius(planet);
    const auto p = ProfileFor(planet);
    const float dominance = c.radiusWorld * p.localDominanceRadiusMultiplier;
    c.dominant = nearestMajor && c.distanceWorld <= dominance;
    c.renderMajorDisc = c.dominant || c.distanceWorld <= dominance * 1.65f;
    const float normalized = c.distanceWorld / std::max(1.0f, dominance);
    c.hazeStrength = std::clamp(1.0f - normalized, 0.0f, 1.0f) * (p.supportsDenseRingField ? 0.42f : 0.22f);
    return c;
}

} // namespace subspace
