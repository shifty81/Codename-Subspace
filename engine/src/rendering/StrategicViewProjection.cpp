#include "rendering/StrategicViewProjection.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kPi = 3.14159265358979323846f;

float Dot(const Vector3& a, const Vector3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

Vector3 SafeNormalize(const Vector3& v, const Vector3& fallback) {
    const float len = v.length();
    if (len <= 1.0e-6f) return fallback;
    return v * (1.0f / len);
}
}

StrategicViewBasis StrategicViewProjection::Build(
    const StrategicCamera& camera,
    float viewportWidth,
    float viewportHeight,
    const StrategicViewProjectionConfig& config)
{
    StrategicViewBasis out;
    out.aspect = std::max(0.1f, viewportWidth / std::max(1.0f, viewportHeight));

    const float fov = std::clamp(config.verticalFovDegrees, 18.0f, 90.0f);
    out.tanHalfFov = std::tan(fov * 0.5f * kPi / 180.0f);
    out.nearPlane = std::max(0.05f, config.nearPlane);
    out.farPlane = std::max(out.nearPlane + 1.0f, config.farPlane);

    if (camera.HasEditorView()) {
        out.eye = camera.GetEditorEye();
        out.target = camera.GetEditorTarget();
        out.forward = SafeNormalize(out.target - out.eye, {0.0f, 1.0f, -1.0f});
        Vector3 worldUp{0.0f, 0.0f, 1.0f};
        // Avoid a degenerate basis when looking almost exactly along world Z.
        if (std::fabs(Dot(out.forward, worldUp)) > 0.985f) worldUp = {0.0f, 1.0f, 0.0f};
        out.right = SafeNormalize(Cross(out.forward, worldUp), {1.0f, 0.0f, 0.0f});
        out.up = SafeNormalize(Cross(out.right, out.forward), {0.0f, 0.0f, 1.0f});
        const float roll = camera.GetEditorRollDegrees() * kPi / 180.0f;
        if (std::fabs(roll) > 1.0e-6f) {
            const float c=std::cos(roll), sn=std::sin(roll);
            const Vector3 r=out.right*c + out.up*sn;
            const Vector3 u=out.up*c - out.right*sn;
            out.right=SafeNormalize(r,out.right); out.up=SafeNormalize(u,out.up);
        }
        return out;
    }

    const float zoom = std::max(0.05f, camera.GetZoom());
    const float distance = std::clamp(config.baseDistance / zoom,
                                      config.minDistance,
                                      config.maxDistance);
    const float tilt = std::clamp(camera.GetVisualTilt(), 0.0f, 1.0f);
    float elevationDegrees = camera.HasElevationOverride()
        ? std::clamp(camera.GetElevationOverrideDegrees(), -89.0f, 89.0f)
        : std::clamp(config.baseElevationDegrees + tilt * config.tiltElevationRangeDegrees,
                     22.0f, 78.0f);
    // Inspection mode may orbit beneath the gameplay plane. Keep a small
    // angular floor around zero so the projection never degenerates into a
    // near-parallel ray against the authoritative X/Y plane.
    if (camera.HasElevationOverride() && std::fabs(elevationDegrees) < 2.0f)
        elevationDegrees = elevationDegrees < 0.0f ? -2.0f : 2.0f;
    const float elevation = elevationDegrees * kPi / 180.0f;

    const Vector3 center = camera.GetCenter();
    const float heightScale = std::clamp(0.82f + camera.GetVisualHeight()*0.32f, 0.70f, 1.25f);
    const float horizontal = std::cos(elevation) * distance;
    const float vertical = std::sin(elevation) * distance * heightScale;

    out.target = {center.x, center.y, 0.0f};
    const float yaw = camera.GetVisualYawDegrees() * kPi / 180.0f;
    out.eye = {center.x + std::sin(yaw) * horizontal,
               center.y - std::cos(yaw) * horizontal,
               vertical};
    out.forward = SafeNormalize(out.target - out.eye, {0.0f, 1.0f, -1.0f});
    out.right = SafeNormalize(Cross(out.forward, {0.0f, 0.0f, 1.0f}), {1.0f, 0.0f, 0.0f});
    out.up = SafeNormalize(Cross(out.right, out.forward), {0.0f, 0.0f, 1.0f});
    return out;
}

Vector3 StrategicViewProjection::ScreenToGameplayPlane(
    float screenX,
    float screenY,
    float viewportWidth,
    float viewportHeight,
    const StrategicCamera& camera,
    float gameplayPlaneZ,
    const StrategicViewProjectionConfig& config)
{
    const StrategicViewBasis basis = Build(camera, viewportWidth, viewportHeight, config);
    const float nx = viewportWidth > 0.0f ? (2.0f*screenX/viewportWidth - 1.0f) : 0.0f;
    const float ny = viewportHeight > 0.0f ? (1.0f - 2.0f*screenY/viewportHeight) : 0.0f;

    const Vector3 ray = SafeNormalize(
        basis.forward + basis.right*(nx*basis.tanHalfFov*basis.aspect) +
        basis.up*(ny*basis.tanHalfFov),
        basis.forward);

    if (std::fabs(ray.z) <= 1.0e-6f) {
        return {basis.target.x, basis.target.y, gameplayPlaneZ};
    }

    const float t = (gameplayPlaneZ - basis.eye.z) / ray.z;
    if (t <= 0.0f) {
        return {basis.target.x, basis.target.y, gameplayPlaneZ};
    }
    const Vector3 hit = basis.eye + ray*t;
    return {hit.x, hit.y, gameplayPlaneZ};
}

StrategicScreenPoint StrategicViewProjection::WorldToScreen(
    const Vector3& world,
    float viewportWidth,
    float viewportHeight,
    const StrategicCamera& camera,
    const StrategicViewProjectionConfig& config)
{
    StrategicScreenPoint out;
    const StrategicViewBasis basis = Build(camera, viewportWidth, viewportHeight, config);
    const Vector3 rel = world - basis.eye;
    const float depth = Dot(rel, basis.forward);
    out.depth = depth;
    if (depth <= basis.nearPlane) return out;

    const float xCamera = Dot(rel, basis.right);
    const float yCamera = Dot(rel, basis.up);
    const float ndcX = xCamera / (depth*basis.tanHalfFov*basis.aspect);
    const float ndcY = yCamera / (depth*basis.tanHalfFov);

    out.x = (ndcX*0.5f + 0.5f)*viewportWidth;
    out.y = (0.5f - ndcY*0.5f)*viewportHeight;
    out.visible = ndcX >= -1.15f && ndcX <= 1.15f && ndcY >= -1.15f && ndcY <= 1.15f && depth <= basis.farPlane;
    return out;
}

} // namespace subspace
