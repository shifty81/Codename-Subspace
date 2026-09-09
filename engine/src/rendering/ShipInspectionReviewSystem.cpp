#include "rendering/ShipInspectionReviewSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {

void ShipInspectionReviewSystem::Orbit(ShipInspectionCameraState& s,float dyaw,float dpitch) const {
    s.snap=ShipInspectionSnap::Free;
    s.yawDegrees=std::fmod(s.yawDegrees+dyaw,360.0f);
    if(s.yawDegrees<0)s.yawDegrees+=360.0f;
    s.pitchDegrees=std::clamp(s.pitchDegrees+dpitch,-89.0f,89.0f);
}
void ShipInspectionReviewSystem::Pan(ShipInspectionCameraState& s,float dx,float dy) const {
    s.pan.x=std::clamp(s.pan.x+dx,-80.0f,80.0f);
    s.pan.y=std::clamp(s.pan.y+dy,-80.0f,80.0f);
}
void ShipInspectionReviewSystem::Zoom(ShipInspectionCameraState& s,float wheel) const {
    s.distance=std::clamp(s.distance-wheel*1.8f,2.2f,220.0f);
}
void ShipInspectionReviewSystem::Snap(ShipInspectionCameraState& s,ShipInspectionSnap snap) const {
    s.snap=snap;
    switch(snap){
        case ShipInspectionSnap::Front:s.yawDegrees=180;s.pitchDegrees=0;break;
        case ShipInspectionSnap::Rear:s.yawDegrees=0;s.pitchDegrees=0;break;
        case ShipInspectionSnap::Left:s.yawDegrees=90;s.pitchDegrees=0;break;
        case ShipInspectionSnap::Right:s.yawDegrees=270;s.pitchDegrees=0;break;
        case ShipInspectionSnap::Top:s.yawDegrees=0;s.pitchDegrees=89;break;
        case ShipInspectionSnap::Bottom:s.yawDegrees=0;s.pitchDegrees=-89;break;
        case ShipInspectionSnap::FrontQuarter:s.yawDegrees=145;s.pitchDegrees=28;break;
        case ShipInspectionSnap::RearQuarter:s.yawDegrees=35;s.pitchDegrees=24;break;
        default:break;
    }
}
std::vector<std::string> ShipInspectionReviewSystem::OverlayLegend(const ShipInspectionOverlay& o) const {
    std::vector<std::string> out;
    if(o.sockets)out.push_back("SOCKETS"); if(o.moduleBounds)out.push_back("MODULE BOUNDS");
    if(o.attachmentLines)out.push_back("ATTACHMENT PATHS"); if(o.thrustVectors)out.push_back("THRUST VECTORS");
    if(o.invalidModules)out.push_back("INVALID / FLOATING"); return out;
}

} // namespace subspace
