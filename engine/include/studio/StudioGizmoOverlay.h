#pragma once
#include "studio/StudioAxisGizmo.h"
namespace subspace {
// Native OpenGL presentation adapter; selection and transform authority stay
// in StudioAxisGizmo and ShipyardBuilderSystem. No second GUI/event host.
struct StudioGizmoOverlay {
    static void Draw(const StudioGizmoSnapshot& layout,int width,int height,
                     StudioAxis active,StudioAxis hovered,bool rotating,float angleDegrees);
};
}
