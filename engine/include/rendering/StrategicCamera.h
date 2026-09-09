#pragma once

#include "core/Math.h"

namespace subspace {

/// Native camera authority for the faux-3D strategic battlefield.
/// Gameplay coordinates remain X/Y; camera height/tilt are presentation
/// parameters consumed by render projection and never fed back into physics.
class StrategicCamera {
public:
    struct ViewBounds {
        float left = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
        float top = 0.0f;
    };

    StrategicCamera() = default;
    explicit StrategicCamera(const Vector3& center) : _center(center), _targetCenter(center) {
        _center.z = 0.0f;
        _targetCenter.z = 0.0f;
    }

    void SetCenter(const Vector3& center);
    const Vector3& GetCenter() const { return _center; }

    void SetTargetCenter(const Vector3& center);
    const Vector3& GetTargetCenter() const { return _targetCenter; }

    void SetZoom(float zoom);
    float GetZoom() const { return _zoom; }

    void SetTargetZoom(float zoom);
    float GetTargetZoom() const { return _targetZoom; }

    void SetZoomLimits(float minZoom, float maxZoom);
    float GetMinZoom() const { return _minZoom; }
    float GetMaxZoom() const { return _maxZoom; }

    void SetFollowSmoothness(float smoothness) { _followSmoothness = smoothness; }
    float GetFollowSmoothness() const { return _followSmoothness; }

    void SetVelocityLookAhead(float seconds) { _velocityLookAhead = seconds; }
    float GetVelocityLookAhead() const { return _velocityLookAhead; }

    void SetVisualTilt(float tilt) { _visualTilt = tilt; }
    float GetVisualTilt() const { return _visualTilt; }

    void SetVisualHeight(float height) { _visualHeight = height; }
    float GetVisualHeight() const { return _visualHeight; }

    /// Optional presentation-only camera elevation override. Negative values
    /// restore the normal strategic tilt model. Vector travel and ship
    /// inspection use this to reach a true over-the-shoulder perspective.
    void SetElevationOverrideDegrees(float degrees) { _elevationOverrideDegrees = degrees; _hasElevationOverride = true; }
    void ClearElevationOverride() { _hasElevationOverride = false; }
    bool HasElevationOverride() const { return _hasElevationOverride; }
    float GetElevationOverrideDegrees() const { return _elevationOverrideDegrees; }


    /// Construction/editor camera override. When active, perspective projection
    /// uses an explicit eye/target/roll rather than deriving an orbital eye from
    /// center/yaw/elevation. Gameplay physics remain unaffected.
    void SetEditorView(const Vector3& eye, const Vector3& target, float rollDegrees = 0.0f) {
        _editorViewEye = eye; _editorViewTarget = target; _editorViewRollDegrees = rollDegrees; _editorViewOverride = true;
    }
    void ClearEditorView() { _editorViewOverride = false; }
    bool HasEditorView() const { return _editorViewOverride; }
    const Vector3& GetEditorEye() const { return _editorViewEye; }
    const Vector3& GetEditorTarget() const { return _editorViewTarget; }
    float GetEditorRollDegrees() const { return _editorViewRollDegrees; }

    /// Horizontal presentation orbit in degrees. This never mutates ship heading or gameplay coordinates.
    void SetVisualYawDegrees(float yawDegrees);
    float GetVisualYawDegrees() const { return _visualYawDegrees; }
    void OrbitVisual(float yawDeltaDegrees, float tiltDelta);

    /// Follow an entity while looking slightly ahead along its velocity.
    void FollowTarget(const Vector3& targetPosition,
                      const Vector3& targetVelocity,
                      float deltaTime);

    /// Smooth current center/zoom toward their targets.
    void Update(float deltaTime);

    /// Pan in gameplay-space units.
    void Pan(const Vector3& delta);

    /// Pan using the current presentation yaw so screen-space MMB movement
    /// remains camera-relative after orbiting around a ship. dx/dy are already
    /// scaled gameplay deltas; positive dx/dy represent pointer motion right/down.
    void PanViewRelative(float dx, float dy);

    /// Planar screen basis used by Shipyard manipulation. These are presentation
    /// helpers only and never rotate the authoritative gameplay plane.
    Vector3 ViewRightPlanar() const;
    Vector3 ViewUpPlanar() const;
    void ClearPanOffset();
    const Vector3& GetPanOffset() const { return _panOffset; }

    /// Multiplicative zoom input. Positive values zoom in.
    void ZoomBy(float amount);

    /// Compute orthographic gameplay bounds for culling/picking.
    ViewBounds GetViewBounds(float viewportWidth, float viewportHeight,
                             float pixelsPerWorldUnit = 32.0f) const;

private:
    static float SmoothFactor(float smoothness, float deltaTime);

    Vector3 _center{};
    Vector3 _targetCenter{};
    Vector3 _panOffset{};
    float _zoom = 1.0f;
    float _targetZoom = 1.0f;
    float _minZoom = 0.20f;
    float _maxZoom = 8.0f;
    float _followSmoothness = 7.0f;
    float _zoomSmoothness = 10.0f;
    float _velocityLookAhead = 0.35f;
    float _visualTilt = 0.32f;
    float _visualHeight = 0.55f;
    float _visualYawDegrees = 0.0f;
    float _elevationOverrideDegrees = 34.0f;
    bool _hasElevationOverride = false;
    bool _editorViewOverride = false;
    Vector3 _editorViewEye{};
    Vector3 _editorViewTarget{};
    float _editorViewRollDegrees = 0.0f;
};

} // namespace subspace
