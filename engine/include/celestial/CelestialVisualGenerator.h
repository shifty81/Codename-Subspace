#pragma once

#include "celestial/CelestialTypes.h"
#include "rendering/RuntimeVisualProfile.h"

namespace subspace {

struct CelestialVisualOptions {
    bool includeOrbitRing = false;
    bool includeGameplayTagAnchor = false;
    float orbitRingScale = 1.0f;
};

RuntimeVisualProfile BuildCelestialVisualProfile(const CelestialBodyDefinition& body,
                                                 const CelestialVisualOptions& options = {});
RuntimeVisualProfile BuildStarSystemBackdropVisualProfile(const StarSystemDefinition& system,
                                                         const CelestialVisualOptions& options = {});

} // namespace subspace
