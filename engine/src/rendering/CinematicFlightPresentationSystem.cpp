#include "rendering/CinematicFlightPresentationSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
float Smooth(float x) {
    x=std::clamp(x,0.0f,1.0f);
    return x*x*(3.0f-2.0f*x);
}
}

VectorCinematicCameraProfile CinematicFlightPresentationSystem::Evaluate(
    VectorTravelStage stage,double stageProgress,double arrivalElapsedSeconds) {
    const float p=static_cast<float>(std::clamp(stageProgress,0.0,1.0));
    VectorCinematicCameraProfile out;
    switch(stage){
        case VectorTravelStage::Aligning:
            out.chaseBlend=Smooth(p)*0.62f;
            out.elevationDegrees=38.0f-(12.0f*Smooth(p));
            out.targetZoom=1.18f+0.34f*Smooth(p);
            out.cameraEase=4.2f;
            break;
        case VectorTravelStage::Charging:
            out.chaseBlend=0.62f+0.38f*Smooth(p);
            out.elevationDegrees=26.0f-(4.0f*Smooth(p));
            out.targetZoom=1.52f+0.38f*Smooth(p);
            out.cameraEase=5.2f;
            break;
        case VectorTravelStage::Cruise:
            out.chaseBlend=1.0f;
            out.elevationDegrees=22.0f;
            out.targetZoom=1.92f;
            out.cameraEase=7.0f;
            break;
        case VectorTravelStage::Decelerating:
            out.chaseBlend=1.0f-0.42f*Smooth(p);
            out.elevationDegrees=22.0f+8.0f*Smooth(p);
            out.targetZoom=1.92f-0.42f*Smooth(p);
            out.arrivalBlend=Smooth(p);
            out.cameraEase=4.8f;
            break;
        case VectorTravelStage::Complete: {
            const float t=static_cast<float>(std::max(0.0,arrivalElapsedSeconds));
            out.chaseBlend=std::max(0.0f,0.58f-t/2.6f);
            out.elevationDegrees=30.0f+std::min(1.0f,t/2.3f)*10.0f;
            out.targetZoom=1.50f-std::min(1.0f,t/2.3f)*0.24f;
            out.arrivalBlend=std::min(1.0f,t/2.3f);
            const float fadeIn=std::clamp(t/0.32f,0.0f,1.0f);
            const float fadeOut=1.0f-std::clamp((t-2.25f)/1.45f,0.0f,1.0f);
            out.arrivalTitleAlpha=Smooth(fadeIn)*Smooth(fadeOut);
            out.cameraEase=3.4f;
            break;
        }
        default: break;
    }
    return out;
}

} // namespace subspace
