#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace subspace {

enum class CameraMode { ShipFlight, TacticalFleet, DockedHangar, ShipBuilder, StationBuilder, PlanetaryManufacturing, SystemMap, OnFoot };
enum class FogKind { DistanceHaze, Nebula, PlanetAtmosphere, RingDust, MiningDust, BattleDebris, IonStorm };

struct CameraProfile {
    CameraMode mode = CameraMode::ShipFlight;
    double yawDegrees = 0.0;
    double pitchDegrees = 55.0;
    double zoom = 1.0;
    double minZoom = 0.2;
    double maxZoom = 5.0;
    double minPitch = 15.0;
    double maxPitch = 80.0;
    bool freePan = false;
};

struct CameraMotionTuning {
    float followSmoothness = 7.0f;
    float velocityLookAhead = 0.24f;
    float recommendedZoom = 0.56f;
    float visualTilt = 0.46f;
};

struct FogLayer {
    FogKind kind = FogKind::DistanceHaze;
    double density = 0.0;
    double visualAttenuation = 0.0;
    double sensorInterference = 0.0;
    double lightScatter = 0.0;
};

class EnvironmentPresentationSystem {
public:
    CameraProfile DefaultCamera(CameraMode mode) const;
    CameraMotionTuning MotionFor(CameraMode mode, float speed) const;
    void Orbit(CameraProfile& camera, double yawDelta, double pitchDelta) const;
    void Zoom(CameraProfile& camera, double delta) const;
    double CombinedVisualAttenuation(const std::vector<FogLayer>& layers) const;
    double CombinedSensorInterference(const std::vector<FogLayer>& layers) const;
    std::vector<FogLayer> RingMiningPreset() const;
    std::vector<FogLayer> IonNebulaPreset() const;
};

} // namespace subspace
