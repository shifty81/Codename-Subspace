#pragma once

#include <cstdint>
#include <string>

namespace subspace {

constexpr double kKilometer = 1000.0;
constexpr double kMegameter = 1000000.0;
constexpr double kAstronomicalUnit = 149597870700.0;

struct AstronomicalPosition {
    std::int64_t regionX = 0;
    std::int64_t regionY = 0;
    double localX = 0.0;
    double localY = 0.0;
};

struct CelestialScaleProfile {
    double physicalRadiusMeters = 1000.0;
    double minimumPresentationRadius = 0.02;
    double maximumPresentationRadius = 0.95;
    bool astronomicalBackground = true;
};

class AstronomicalScaleSystem {
public:
    static constexpr double RegionSizeMeters() { return 250000000.0; }
    AstronomicalPosition Normalize(const AstronomicalPosition& value) const;
    double DistanceMeters(const AstronomicalPosition& a, const AstronomicalPosition& b) const;
    double PresentationRadius(const CelestialScaleProfile& profile, double distanceMeters) const;
    CelestialScaleProfile PlanetProfile(double physicalRadiusMeters) const;
    CelestialScaleProfile StarProfile(double physicalRadiusMeters) const;
};

} // namespace subspace
