#include "station/StationNavigationLightSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

std::vector<StationNavLight> StationNavigationLightSystem::Build(const StationDockGeometry& d,bool clearance,bool occupied) const {
    std::vector<StationNavLight> out;
    if(d.corridor.empty())return out;
    Vector3 side{-d.approachDirection.y,d.approachDirection.x,0.0f};
    for(std::size_t segment=0;segment+1<d.corridor.size();++segment){
        const auto& a=d.corridor[segment].position;const auto& b=d.corridor[segment+1].position;
        for(int i=0;i<4;++i){
            const float t=(float(i)+.25f)/4.0f;Vector3 p=a+(b-a)*t;
            const float width=1.7f+float(segment)*-.25f;
            DockNavLightKind kind=occupied?DockNavLightKind::Closed:(clearance?DockNavLightKind::Guidance:DockNavLightKind::Hold);
            out.push_back({p+side*width,kind,float(segment*4+i)*.13f,1.0f});
            out.push_back({p-side*width,kind,float(segment*4+i)*.13f+.06f,1.0f});
        }
    }
    out.push_back({d.apertureWorld+side*1.35f,occupied?DockNavLightKind::Closed:DockNavLightKind::Aperture,.0f,1.0f});
    out.push_back({d.apertureWorld-side*1.35f,occupied?DockNavLightKind::Closed:DockNavLightKind::Aperture,.5f,1.0f});
    // A sparse ring describes the docking envelope without drawing a solid bubble.
    for(int i=0;i<16;++i){
        constexpr float kPi=3.14159265358979323846f;const float a=float(i)/16.0f*2.0f*kPi;
        out.push_back({d.stationWorld+Vector3{std::cos(a)*d.dockingEnvelopeRadius,std::sin(a)*d.dockingEnvelopeRadius,0.0f},DockNavLightKind::ServiceBoundary,float(i)/16.0f,.42f});
    }
    return out;
}

float StationNavigationLightSystem::Pulse(const StationNavLight& light,double seconds){
    const float phase=static_cast<float>(seconds)*2.8f-light.phaseOffset*6.2831853f;
    const float wave=.5f+.5f*std::sin(phase);
    return std::clamp((.22f+.78f*wave)*light.intensity,0.0f,1.0f);
}

} // namespace subspace
