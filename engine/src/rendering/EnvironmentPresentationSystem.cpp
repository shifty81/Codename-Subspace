#include "rendering/EnvironmentPresentationSystem.h"

#include <cmath>

namespace subspace {

CameraProfile EnvironmentPresentationSystem::DefaultCamera(CameraMode mode) const {
    CameraProfile c; c.mode=mode;
    switch(mode){
        case CameraMode::ShipFlight: c={mode,0,55,1.0,0.18,7.0,22,78,false}; break;
        case CameraMode::TacticalFleet: c={mode,0,65,1.0,0.08,16.0,30,86,true}; break;
        case CameraMode::DockedHangar: c={mode,0,42,1.0,0.35,4.0,10,82,false}; break;
        case CameraMode::ShipBuilder: case CameraMode::StationBuilder: c={mode,0,48,1.0,0.12,12.0,5,88,true}; break;
        case CameraMode::PlanetaryManufacturing: c={mode,0,50,1.0,0.12,9.0,20,80,true}; break;
        case CameraMode::SystemMap: c={mode,0,72,1.0,0.02,40.0,25,89,true}; break;
        case CameraMode::OnFoot: c={mode,0,58,1.0,0.82,1.45,50,66,false}; break;
    }
    return c;
}

CameraMotionTuning EnvironmentPresentationSystem::MotionFor(CameraMode mode,float speed) const {
    const float speed01=std::clamp(speed/90.0f,0.0f,1.0f);
    switch(mode){
        case CameraMode::OnFoot: return {10.5f,0.0f,1.12f,0.72f};
        case CameraMode::DockedHangar: return {9.5f,0.0f,0.82f,0.58f};
        case CameraMode::TacticalFleet: return {5.4f,0.12f,0.42f,0.40f};
        case CameraMode::ShipBuilder: case CameraMode::StationBuilder: return {8.5f,0.0f,1.0f,0.50f};
        case CameraMode::SystemMap: return {12.0f,0.0f,0.30f,0.25f};
        default: return {7.6f-1.4f*speed01,0.18f+0.34f*speed01,0.56f-0.10f*speed01,0.46f};
    }
}
void EnvironmentPresentationSystem::Orbit(CameraProfile& c,double yawDelta,double pitchDelta) const {
    c.yawDegrees=std::fmod(c.yawDegrees+yawDelta,360.0); if(c.yawDegrees<0)c.yawDegrees+=360.0;
    c.pitchDegrees=std::clamp(c.pitchDegrees+pitchDelta,c.minPitch,c.maxPitch);
}
void EnvironmentPresentationSystem::Zoom(CameraProfile& c,double delta) const { c.zoom=std::clamp(c.zoom+delta,c.minZoom,c.maxZoom); }
double EnvironmentPresentationSystem::CombinedVisualAttenuation(const std::vector<FogLayer>& layers) const { double remain=1.0; for(const auto& l:layers) remain*=1.0-std::clamp(l.visualAttenuation,0.0,1.0); return 1.0-remain; }
double EnvironmentPresentationSystem::CombinedSensorInterference(const std::vector<FogLayer>& layers) const { double remain=1.0; for(const auto& l:layers) remain*=1.0-std::clamp(l.sensorInterference,0.0,1.0); return 1.0-remain; }
std::vector<FogLayer> EnvironmentPresentationSystem::RingMiningPreset() const { return {{FogKind::DistanceHaze,.15,.10,.02,.12},{FogKind::RingDust,.35,.28,.08,.35},{FogKind::MiningDust,.25,.18,.03,.18}}; }
std::vector<FogLayer> EnvironmentPresentationSystem::IonNebulaPreset() const { return {{FogKind::Nebula,.45,.24,.35,.38},{FogKind::IonStorm,.20,.05,.55,.10}}; }

} // namespace subspace
