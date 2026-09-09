#include "rendering/StrategicCamera.h"

#include <algorithm>
#include <cmath>

namespace subspace {

float StrategicCamera::SmoothFactor(float smoothness, float deltaTime)
{
    if (deltaTime <= 0.0f) return 0.0f;
    return 1.0f - std::exp(-std::max(0.0f, smoothness) * deltaTime);
}

void StrategicCamera::SetCenter(const Vector3& center)
{
    _center = {center.x, center.y, 0.0f};
    _targetCenter = _center;
}

void StrategicCamera::SetTargetCenter(const Vector3& center)
{
    _targetCenter = {center.x, center.y, 0.0f};
}

void StrategicCamera::SetZoom(float zoom)
{
    _zoom = std::clamp(zoom, _minZoom, _maxZoom);
    _targetZoom = _zoom;
}

void StrategicCamera::SetTargetZoom(float zoom)
{
    _targetZoom = std::clamp(zoom, _minZoom, _maxZoom);
}

void StrategicCamera::SetZoomLimits(float minZoom, float maxZoom)
{
    if (minZoom <= 0.0f) minZoom = 0.01f;
    if (maxZoom < minZoom) std::swap(minZoom, maxZoom);
    _minZoom = minZoom;
    _maxZoom = maxZoom;
    _zoom = std::clamp(_zoom, _minZoom, _maxZoom);
    _targetZoom = std::clamp(_targetZoom, _minZoom, _maxZoom);
}

void StrategicCamera::SetVisualYawDegrees(float yawDegrees)
{
    _visualYawDegrees = std::fmod(yawDegrees, 360.0f);
    if (_visualYawDegrees < 0.0f) _visualYawDegrees += 360.0f;
}

void StrategicCamera::OrbitVisual(float yawDeltaDegrees, float tiltDelta)
{
    SetVisualYawDegrees(_visualYawDegrees + yawDeltaDegrees);
    _visualTilt = std::clamp(_visualTilt + tiltDelta, 0.0f, 1.0f);
}

void StrategicCamera::FollowTarget(const Vector3& targetPosition,
                                   const Vector3& targetVelocity,
                                   float deltaTime)
{
    // Player-facing flight keeps the piloted subject visually anchored instead
    // of allowing follow smoothing / velocity look-ahead to slide the ship
    // around the viewport.  Preserve only a tiny directional cue (8 cm world
    // maximum) so heading/motion still reads without changing composition.
    constexpr float kMaxScreenLookAhead = 0.08f;
    Vector3 lookAhead{
        targetVelocity.x * _velocityLookAhead,
        targetVelocity.y * _velocityLookAhead,
        0.0f
    };
    const float lookLen=lookAhead.length();
    if(lookLen>kMaxScreenLookAhead)lookAhead=lookAhead*(kMaxScreenLookAhead/lookLen);
    _targetCenter={targetPosition.x+lookAhead.x+_panOffset.x,
                   targetPosition.y+lookAhead.y+_panOffset.y,0.0f};
    _center=_targetCenter;

    // Position is screen-locked; zoom remains smoothly animated.
    const float zoomAlpha = SmoothFactor(_zoomSmoothness, deltaTime);
    _zoom += (_targetZoom - _zoom) * zoomAlpha;
    _zoom = std::clamp(_zoom, _minZoom, _maxZoom);
}

void StrategicCamera::Update(float deltaTime)
{
    const float followAlpha = SmoothFactor(_followSmoothness, deltaTime);
    _center = _center + (_targetCenter - _center) * followAlpha;
    _center.z = 0.0f;

    const float zoomAlpha = SmoothFactor(_zoomSmoothness, deltaTime);
    _zoom += (_targetZoom - _zoom) * zoomAlpha;
    _zoom = std::clamp(_zoom, _minZoom, _maxZoom);
}

void StrategicCamera::Pan(const Vector3& delta)
{
    const Vector3 planarDelta{delta.x, delta.y, 0.0f};
    _panOffset = _panOffset + planarDelta;
    _center = _center + planarDelta;
    _targetCenter = _targetCenter + planarDelta;
}

Vector3 StrategicCamera::ViewRightPlanar() const
{
    if(_editorViewOverride){
        const Vector3 d=_editorViewTarget-_editorViewEye;
        const float len=std::sqrt(d.x*d.x+d.y*d.y);
        if(len>1.0e-5f)return {d.y/len,-d.x/len,0.0f};
    }
    constexpr float kPi = 3.14159265358979323846f;
    const float yaw = _visualYawDegrees * kPi / 180.0f;
    return {std::cos(yaw), std::sin(yaw), 0.0f};
}

Vector3 StrategicCamera::ViewUpPlanar() const
{
    if(_editorViewOverride){
        const Vector3 d=_editorViewTarget-_editorViewEye;
        const float len=std::sqrt(d.x*d.x+d.y*d.y);
        if(len>1.0e-5f)return {d.x/len,d.y/len,0.0f};
    }
    constexpr float kPi = 3.14159265358979323846f;
    const float yaw = _visualYawDegrees * kPi / 180.0f;
    return {-std::sin(yaw), std::cos(yaw), 0.0f};
}

void StrategicCamera::PanViewRelative(float dx, float dy)
{
    // Dragging the scene right moves the camera pivot left. Dragging down moves
    // the pivot toward screen-up on the gameplay plane. This preserves the
    // established Shipyard drag feel while making it independent of camera yaw.
    Pan(ViewRightPlanar() * (-dx) + ViewUpPlanar() * dy);
}

void StrategicCamera::ClearPanOffset()
{
    _panOffset = {};
}

void StrategicCamera::ZoomBy(float amount)
{
    const float factor = std::exp(amount * 0.12f);
    SetTargetZoom(_targetZoom * factor);
}

StrategicCamera::ViewBounds StrategicCamera::GetViewBounds(
    float viewportWidth, float viewportHeight, float pixelsPerWorldUnit) const
{
    pixelsPerWorldUnit = std::max(1.0f, pixelsPerWorldUnit);
    const float safeZoom = std::max(_minZoom, _zoom);
    const float halfWidth = viewportWidth / (2.0f * pixelsPerWorldUnit * safeZoom);
    const float halfHeight = viewportHeight / (2.0f * pixelsPerWorldUnit * safeZoom);

    return {
        _center.x - halfWidth,
        _center.x + halfWidth,
        _center.y - halfHeight,
        _center.y + halfHeight
    };
}

} // namespace subspace
