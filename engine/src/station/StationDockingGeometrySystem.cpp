#include "station/StationDockingGeometrySystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
float Dot(const Vector3& a,const Vector3& b){return a.x*b.x+a.y*b.y+a.z*b.z;}
float Distance(const Vector3& a,const Vector3& b){return (a-b).length();}
Vector3 PlanarNormal(const Vector3& v){Vector3 p{v.x,v.y,0.0f};return p.length()>0.0001f?p.normalized():Vector3{0.0f,-1.0f,0.0f};}
float Clamp01(float v){return std::clamp(v,0.0f,1.0f);}
}

const char* StationDockingGeometrySystem::BerthSizeName(StationBerthSize size){
    switch(size){
        case StationBerthSize::Small:return "SMALL";
        case StationBerthSize::Standard:return "STANDARD";
        case StationBerthSize::Heavy:return "HEAVY";
        case StationBerthSize::Capital:return "CAPITAL";
    }
    return "STANDARD";
}

StationDockGeometry StationDockingGeometrySystem::Build(std::uint64_t stationId,
                                                         const Vector3& station,
                                                         const Vector3& ship,
                                                         StationBerthSize requestedSize) const {
    StationDockGeometry d;d.stationId=stationId;d.stationWorld=station;d.berthSize=requestedSize;
    Vector3 radial=PlanarNormal(ship-station);
    // Snap the berth to one of eight station-facing sectors. This keeps visible
    // docking infrastructure stable while still assigning the closest entrance.
    const float angle=std::atan2(radial.y,radial.x);
    constexpr float kPi=3.14159265358979323846f;
    const float step=kPi/4.0f;
    const float snapped=std::round(angle/step)*step;
    radial={std::cos(snapped),std::sin(snapped),0.0f};
    d.approachDirection=radial;
    const float sizeScale=requestedSize==StationBerthSize::Small?.78f:requestedSize==StationBerthSize::Heavy?1.35f:requestedSize==StationBerthSize::Capital?2.1f:1.0f;
    d.dockingEnvelopeRadius=42.0f*sizeScale;
    d.captureRadius=2.2f*sizeScale;
    d.captureSpeedLimit=6.0f/std::max(.8f,sizeScale);
    d.apertureWorld=station+radial*(9.0f*sizeScale);
    d.captureWorld=station+radial*(12.0f*sizeScale);
    d.undockWorld=station+radial*(26.0f*sizeScale);
    const int berthIndex=static_cast<int>((stationId + static_cast<std::uint64_t>(std::llround((snapped+kPi)*100.0f)))%12u)+1;
    d.berthId="BERTH-"+(berthIndex<10?std::string("0"):std::string{})+std::to_string(berthIndex);
    d.corridor={
        {station+radial*(58.0f*sizeScale),7.5f*sizeScale,28.0f},
        {station+radial*(40.0f*sizeScale),6.0f*sizeScale,20.0f},
        {station+radial*(24.0f*sizeScale),4.4f*sizeScale,12.0f},
        {d.captureWorld,d.captureRadius*1.55f,d.captureSpeedLimit}
    };
    return d;
}

DockCaptureEvaluation StationDockingGeometrySystem::Evaluate(const StationDockGeometry& dock,
                                                              const Vector3& ship,
                                                              const Vector3& velocity,
                                                              const Vector3& forward) const {
    DockCaptureEvaluation e;
    e.distanceToCapture=Distance(ship,dock.captureWorld);
    e.insideEnvelope=Distance(ship,dock.stationWorld)<=dock.dockingEnvelopeRadius;
    e.insideCapture=e.distanceToCapture<=dock.captureRadius;
    e.speedAcceptable=velocity.length()<=dock.captureSpeedLimit;
    const Vector3 desired=(dock.apertureWorld-dock.captureWorld).normalized();
    const Vector3 f=forward.length()>0.0001f?forward.normalized():desired;
    e.alignment=Clamp01((Dot(f,desired)+1.0f)*0.5f);
    e.capturable=e.insideCapture&&e.speedAcceptable&&e.alignment>=.78f;
    return e;
}

} // namespace subspace
