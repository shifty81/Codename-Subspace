#pragma once

#include "core/Math.h"
#include "rendering/StrategicCamera.h"

namespace subspace {

struct StrategicViewProjectionConfig {
    float verticalFovDegrees = 45.0f;
    float baseDistance = 44.0f;
    float minDistance = 1.35f;
    float maxDistance = 220.0f;
    float baseElevationDegrees = 48.0f;
    float tiltElevationRangeDegrees = 16.0f;
    float nearPlane = 0.08f;
    float farPlane = 10000.0f;
};

struct StrategicViewBasis {
    Vector3 eye{};
    Vector3 target{};
    Vector3 forward{};
    Vector3 right{};
    Vector3 up{};
    float aspect = 16.0f / 9.0f;
    float tanHalfFov = 0.383864f;
    float nearPlane = 0.08f;
    float farPlane = 10000.0f;
};

struct StrategicScreenPoint {
    float x = 0.0f;
    float y = 0.0f;
    float depth = 0.0f;
    bool visible = false;
};

/// Pure-math perspective authority for the faux-3D strategic view.
///
/// The camera may sit above and behind the authoritative X/Y gameplay plane,
/// but all picking explicitly intersects that plane again.  Presentation Z
/// therefore cannot leak into ship physics or world simulation.
class StrategicViewProjection {
public:
    static StrategicViewBasis Build(const StrategicCamera& camera,
                                    float viewportWidth,
                                    float viewportHeight,
                                    const StrategicViewProjectionConfig& config = {});

    static Vector3 ScreenToGameplayPlane(float screenX,
                                         float screenY,
                                         float viewportWidth,
                                         float viewportHeight,
                                         const StrategicCamera& camera,
                                         float gameplayPlaneZ = 0.0f,
                                         const StrategicViewProjectionConfig& config = {});

    static StrategicScreenPoint WorldToScreen(const Vector3& world,
                                               float viewportWidth,
                                               float viewportHeight,
                                               const StrategicCamera& camera,
                                               const StrategicViewProjectionConfig& config = {});
};

} // namespace subspace
