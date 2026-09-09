#pragma once

#include "procedural/GalaxyGenerator.h"

#include <string>
#include <vector>

namespace subspace {

struct SolarSystemPlacementReport {
    bool certified = true;
    int violations = 0;
    int repairs = 0;
    std::vector<std::string> issues;
};

/// Canonical macro-system placement authority.
///
/// Galaxy generation is allowed to create deterministic content records, but
/// nothing is considered spawn-certified until it passes this policy. The
/// authority keeps ordinary gameplay objects out of stellar/planet/ring
/// envelopes and assigns legacy origin-centered content to semantically useful
/// belts, planetary neighborhoods, station traffic regions, or deep space.
class SolarSystemPlacementSystem {
public:
    float StellarExclusionRadiusSector(const StarData& star) const;
    float PlanetEnvelopeRadiusSector(const PlanetData& planet) const;
    float OuterSystemRadius(const GalaxySector& sector) const;

    /// Runtime/local-materialization safety bridge. Regional streaming and
    /// authored encounter code may reposition otherwise valid contacts after
    /// generation; those positions still have to respect the same stellar and
    /// celestial envelopes as world generation.
    SectorPosition RepairSpawnPosition(const GalaxySector& sector, SectorPosition position,
                                       float stellarMargin = 12000.0f,
                                       float planetMargin = 8000.0f) const;
    bool IsSpawnPositionSafe(const GalaxySector& sector, const SectorPosition& position,
                             float stellarMargin = 0.0f,
                             float planetMargin = 0.0f) const;

    SolarSystemPlacementReport Normalize(GalaxySector& sector) const;
    SolarSystemPlacementReport Validate(const GalaxySector& sector) const;
};

} // namespace subspace
