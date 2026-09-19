#pragma once
#include "editor/ConstructionEditorCameraSystem.h"
#include <cmath>
namespace subspace {
// Studio input adapter, NOT an alternate camera or ship transform authority.
// All camera position/perspective is owned by ConstructionEditorCameraSystem.
class StudioCameraNavigation {
public:
    static void Orbit(ConstructionEditorCameraState& camera,float pixelX,float pixelY,bool rollModifier){
        if(!std::isfinite(pixelX)||!std::isfinite(pixelY))return;
        if(rollModifier){
            // The shared Roll() method enters FreeFly; orbital roll must stay
            // centered on the assembly instead of unexpectedly moving its pivot.
            camera.rollDegrees=std::remainder(camera.rollDegrees+pixelX*.38f,360.0f);
            ConstructionEditorCameraSystem::Orbit(camera,0.0f,0.0f);
        }else{
            ConstructionEditorCameraSystem::Orbit(camera,pixelX*.38f,-pixelY*.38f);
        }
    }
};
} // namespace subspace
