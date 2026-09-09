#include "navigation/AstronomicalScaleSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

AstronomicalPosition AstronomicalScaleSystem::Normalize(const AstronomicalPosition& value) const {
    AstronomicalPosition out = value;
    const double size = RegionSizeMeters();
    while (out.localX >= size * 0.5) { out.localX -= size; ++out.regionX; }
    while (out.localX < -size * 0.5) { out.localX += size; --out.regionX; }
    while (out.localY >= size * 0.5) { out.localY -= size; ++out.regionY; }
    while (out.localY < -size * 0.5) { out.localY += size; --out.regionY; }
    return out;
}

double AstronomicalScaleSystem::DistanceMeters(const AstronomicalPosition& a, const AstronomicalPosition& b) const {
    const double dx = static_cast<double>(a.regionX - b.regionX) * RegionSizeMeters() + (a.localX - b.localX);
    const double dy = static_cast<double>(a.regionY - b.regionY) * RegionSizeMeters() + (a.localY - b.localY);
    return std::sqrt(dx * dx + dy * dy);
}

double AstronomicalScaleSystem::PresentationRadius(const CelestialScaleProfile& profile, double distanceMeters) const {
    const double safeDistance = std::max(distanceMeters, profile.physicalRadiusMeters * 1.001);
    const double angular = std::atan2(profile.physicalRadiusMeters, safeDistance);
    const double normalized = angular / 0.7853981633974483;
    return std::clamp(normalized, profile.minimumPresentationRadius, profile.maximumPresentationRadius);
}

CelestialScaleProfile AstronomicalScaleSystem::PlanetProfile(double physicalRadiusMeters) const {
    return {std::max(physicalRadiusMeters, 100000.0), 0.015, 0.98, true};
}

CelestialScaleProfile AstronomicalScaleSystem::StarProfile(double physicalRadiusMeters) const {
    return {std::max(physicalRadiusMeters, 100000000.0), 0.025, 1.25, true};
}

} // namespace subspace
