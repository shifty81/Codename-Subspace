#pragma once
#include <algorithm>
#include <cmath>

namespace subspace {
// One editor manipulation contract: X=starboard/pitch, Y=forward/roll,
// Z=dorsal/yaw in the existing renderer Euler order Rz(yaw)*Rx(pitch)*Ry(roll).
// Camera yaw and recipe forwardVisualYaw are presentation only.
enum class StudioAxis { None=-1, X=0, Y=1, Z=2 };
struct StudioPoint { float x=0, y=0; };
struct StudioAxisHandle {
    StudioAxis axis=StudioAxis::None;
    StudioPoint center{},tip{};
    bool valid=false;
    // Frozen, source-exact derivative of this PHYSICAL axis in pixels per
    // authored unit. UI reflow may move tip, but must retain calibration.
    StudioPoint physicalPixelsPerUnit{};
    float fallbackPixelsPerUnit=0.0f;
    bool projectedAxisUsable=false;
};
struct StudioGizmoMath {
    static float Length(StudioPoint v) noexcept { return std::sqrt(v.x*v.x+v.y*v.y); }
    static StudioPoint Unit(StudioPoint v) noexcept {
        const float length=Length(v);return length>0.0001f?StudioPoint{v.x/length,v.y/length}:StudioPoint{};
    }
    static StudioPoint Delta(StudioPoint a,StudioPoint b) noexcept {return {a.x-b.x,a.y-b.y};}
    static float Dot(StudioPoint a,StudioPoint b) noexcept {return a.x*b.x+a.y*b.y;}
    static float DragScalar(const StudioAxisHandle& h,float dx,float dy,bool rotation) noexcept {
        if(!h.valid)return 0;
        auto u=Unit(Delta(h.tip,h.center));
        if(rotation)u={-u.y,u.x};
        return Dot({dx,dy},u);
    }
    static bool Hit(const StudioAxisHandle& h,float px,float py,float radius=15.0f) noexcept {
        if(!h.valid)return false;
        const auto d=Delta(h.tip,h.center),q=Delta({px,py},h.center);
        const float len2=Dot(d,d);if(len2<64)return false;
        // The pivot is not a selectable axis; prevent X/Y/Z ambiguity.
        const float tipDistance=Length(Delta({px,py},h.tip));
        if(tipDistance<=radius+6.0f)return true; // marker is intentionally easier than the shaft
        // The proximal handle is a real hit target without capturing the
        // shared pivot; keep shaft and tip radii distinct.
        const float t=Dot(q,d)/len2;if(t<.16f||t>1.12f)return false;
        const StudioPoint nearestPoint{h.center.x+d.x*t,h.center.y+d.y*t};
        return Length(Delta({px,py},nearestPoint))<=std::max(8.0f,radius-3.0f);
    }
    static float WrappedAngle(float angle) noexcept {
        if(!std::isfinite(angle))return 0;
        float wrapped=std::fmod(angle,360.0f);
        if(wrapped>180.0f)wrapped-=360.0f;
        if(wrapped<=-180.0f)wrapped+=360.0f;
        return wrapped;
    }
    static float Component(StudioAxis axis,float x,float y,float z) noexcept {
        switch(axis){case StudioAxis::X:return x;case StudioAxis::Y:return y;case StudioAxis::Z:return z;default:return 0;}
    }
    // The persisted placement stores pitch(x), yaw(z), roll(y) as fields,
    // but the renderer applies Rz(yaw)*Rx(pitch)*Ry(roll). Do not equate
    // enum order with struct-field order: visual axis and edited angle agree.
    static float RotationComponent(StudioAxis axis,float pitch,float yaw,float roll) noexcept {
        switch(axis){case StudioAxis::X:return pitch;case StudioAxis::Y:return roll;
                     case StudioAxis::Z:return yaw;default:return 0;}
    }
    // The public gizmo names physical rotation axes. The existing builder
    // constraints instead mask the packed rotation delta: X=pitch, Y=yaw,
    // Z=roll. Map to the FIELD axis before calling RotateSelected, otherwise
    // the physical Y/Z input is discarded by ApplyTransformConstraint.
    static constexpr StudioAxis RotationFieldAxis(StudioAxis axis) noexcept {
        switch(axis){
            case StudioAxis::X:return StudioAxis::X; // pitch delta.x
            case StudioAxis::Y:return StudioAxis::Z; // roll delta.z
            case StudioAxis::Z:return StudioAxis::Y; // yaw delta.y
            default:return StudioAxis::None;
        }
    }
    // The assembled ship and the non-destructive model recipe have distinct
    // rotation storage contracts. Never apply the assembly Y/Z swap to the
    // model's true XYZ angles: that causes a moving angle HUD but no geometry.
    static constexpr StudioAxis ConstraintFieldAxis(StudioAxis axis,bool modelMode,bool rotation) noexcept {
        return rotation&&!modelMode?RotationFieldAxis(axis):axis;
    }
};
}
