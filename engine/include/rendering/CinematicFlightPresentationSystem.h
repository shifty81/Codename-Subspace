#pragma once

#include "navigation/VectorTravelSystem.h"

namespace subspace {

struct VectorCinematicCameraProfile {
    float chaseBlend = 0.0f;
    float elevationDegrees = -1.0f;
    float targetZoom = 1.0f;
    float cameraEase = 6.0f;
    float arrivalBlend = 0.0f;
    float arrivalTitleAlpha = 0.0f;
};

/// Pass355-356 pure presentation authority for Vector entry/cruise/arrival.
/// It keeps travel camera decisions out of the renderer and makes the intended
/// rear-thrust chase composition independently testable.
class CinematicFlightPresentationSystem {
public:
    static VectorCinematicCameraProfile Evaluate(VectorTravelStage stage,
                                                  double stageProgress,
                                                  double arrivalElapsedSeconds = 0.0);
};

} // namespace subspace
