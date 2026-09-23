#pragma once

#include "core/Math.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cmath>

namespace subspace {

struct ConstructionTransformBasis {
    Vector3 x{1,0,0};
    Vector3 y{0,1,0};
    Vector3 z{0,0,1};
};

// One transform-basis authority shared by Studio gizmos and authoring edits.
// Assembly order intentionally matches NativeBattlefieldRenderer:
// local mirror/scale -> roll about Y -> pitch about X -> yaw about Z -> root.
// Model order matches ShipyardModelingSystem::TransformMatrix (Rz * Ry * Rx).
class ConstructionTransformBasisSystem {
public:
    static constexpr float Pi() noexcept { return 3.14159265358979323846f; }
    static constexpr float DegreesToRadians(float d) noexcept { return d*Pi()/180.0f; }

    static Vector3 RotateX(Vector3 v,float r) noexcept {
        const float c=std::cos(r),s=std::sin(r);return {v.x,v.y*c-v.z*s,v.y*s+v.z*c};
    }
    static Vector3 RotateY(Vector3 v,float r) noexcept {
        const float c=std::cos(r),s=std::sin(r);return {v.x*c+v.z*s,v.y,-v.x*s+v.z*c};
    }
    static Vector3 RotateZ(Vector3 v,float r) noexcept {
        const float c=std::cos(r),s=std::sin(r);return {v.x*c-v.y*s,v.x*s+v.y*c,v.z};
    }

    static Vector3 ApplyAssemblyRotation(Vector3 v,const VisualModulePlacement& p) noexcept {
        v=RotateY(v,DegreesToRadians(p.rollDegrees));
        v=RotateX(v,DegreesToRadians(p.pitchDegrees));
        v=RotateZ(v,DegreesToRadians(p.yawDegrees));
        return v;
    }

    static ConstructionTransformBasis AssemblyLocal(const VisualModulePlacement& p,
                                                     bool includeMirror=true) noexcept {
        const float mx=includeMirror&&p.mirrorX?-1.0f:1.0f;
        const float my=includeMirror&&p.mirrorY?-1.0f:1.0f;
        const float mz=includeMirror&&p.mirrorZ?-1.0f:1.0f;
        return {ApplyAssemblyRotation({mx,0,0},p),
                ApplyAssemblyRotation({0,my,0},p),
                ApplyAssemblyRotation({0,0,mz},p)};
    }

    // Returns actual world vectors for one authored local unit. Root scale is
    // applied after module rotation, exactly like the OpenGL hierarchy.
    static ConstructionTransformBasis AssemblyWorld(const VisualModulePlacement& p,
                                                     float rootYawRadians,
                                                     const Vector3& rootScale,
                                                     bool includeMirror=true) noexcept {
        auto b=AssemblyLocal(p,includeMirror);
        auto world=[&](Vector3 v){
            v={v.x*rootScale.x,v.y*rootScale.y,v.z*rootScale.z};
            return RotateZ(v,rootYawRadians);
        };
        b.x=world(b.x);b.y=world(b.y);b.z=world(b.z);return b;
    }

    static ConstructionTransformBasis ShipWorld(float rootYawRadians,
                                                const Vector3& rootScale) noexcept {
        return {RotateZ({rootScale.x,0,0},rootYawRadians),
                RotateZ({0,rootScale.y,0},rootYawRadians),
                {0,0,rootScale.z}};
    }

    static ConstructionTransformBasis ModelLocal(const Vector3& degrees) noexcept {
        auto apply=[&](Vector3 v){
            v=RotateX(v,DegreesToRadians(degrees.x));
            v=RotateY(v,DegreesToRadians(degrees.y));
            v=RotateZ(v,DegreesToRadians(degrees.z));
            return v;
        };
        return {apply({1,0,0}),apply({0,1,0}),apply({0,0,1})};
    }

    static Vector3 Axis(const ConstructionTransformBasis& b,int axis) noexcept {
        return axis==1?b.y:(axis==2?b.z:b.x);
    }

    static Vector3 Normalize(Vector3 v,Vector3 fallback={1,0,0}) noexcept {
        const float m=std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
        if(!std::isfinite(m)||m<1.0e-6f)return fallback;
        return {v.x/m,v.y/m,v.z/m};
    }
};

} // namespace subspace
