#pragma once
#include "studio/StudioGizmoMath.h"
#include "studio/StudioTransformReadout.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "rendering/StrategicCamera.h"
#include <array>

namespace subspace {
struct StudioGizmoSnapshot {
    std::array<StudioAxisHandle,3> handles{};
    float viewportLeft=0,viewportTop=0,viewportRight=0,viewportBottom=0;
    bool visible=false;
    bool readoutVisible=false;
    StudioTransformReadout readout{};
    const StudioAxisHandle* Handle(StudioAxis axis) const noexcept {
        const int index=static_cast<int>(axis);
        return index>=0&&index<3?&handles[static_cast<std::size_t>(index)]:nullptr;
    }
    StudioAxis Pick(float x,float y) const noexcept {
        if(!visible||x<viewportLeft||x>viewportRight||y<viewportTop||y>viewportBottom)return StudioAxis::None;
        // Select closest tip first, then shaft, deterministic ties.
        float best=1e30f;StudioAxis selected=StudioAxis::None;
        for(const auto& h:handles){
            if(!StudioGizmoMath::Hit(h,x,y))continue;
            const float d=StudioGizmoMath::Length(StudioGizmoMath::Delta({x,y},h.tip));
            if(d<best){best=d;selected=h.axis;}
        }
        return selected;
    }
};
class StudioAxisGizmo {
public:
    static StudioGizmoSnapshot Build(const ShipyardBuilderRuntimeModel& model,
                                     const StrategicCamera& camera,int width,int height);
};
}
