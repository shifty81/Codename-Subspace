#pragma once
#include "editor/ConstructionTransformBasisSystem.h"
#include "rendering/StrategicCamera.h"
#include <cmath>

namespace subspace {
// Presentation-only camera basis for VIEW transform orientation. It never
// changes authored coordinates, the camera, or the selected object's rotation.
struct StudioTransformViewBasis {
    static Vector3 Cross(Vector3 a,Vector3 b) noexcept {
        return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
    }
    static float Dot(Vector3 a,Vector3 b) noexcept { return a.x*b.x+a.y*b.y+a.z*b.z; }
    static ConstructionTransformBasis Build(const StrategicCamera& camera) noexcept {
        if(!camera.HasEditorView()){
            const auto right=ConstructionTransformBasisSystem::Normalize(camera.ViewRightPlanar(),{1,0,0});
            const auto up=ConstructionTransformBasisSystem::Normalize(camera.ViewUpPlanar(),{0,1,0});
            return {right,up,{0,0,1}};
        }
        auto forward=ConstructionTransformBasisSystem::Normalize(camera.GetEditorTarget()-camera.GetEditorEye(),{0,1,0});
        Vector3 referenceUp{0,0,1};
        if(std::fabs(Dot(forward,referenceUp))>.985f)referenceUp={0,1,0};
        auto right=ConstructionTransformBasisSystem::Normalize(Cross(forward,referenceUp),{1,0,0});
        auto up=ConstructionTransformBasisSystem::Normalize(Cross(right,forward),{0,0,1});
        const float roll=ConstructionTransformBasisSystem::DegreesToRadians(camera.GetEditorRollDegrees());
        const float c=std::cos(roll),s=std::sin(roll);
        const auto rolledRight=right*c+up*s;
        const auto rolledUp=up*c-right*s;
        // X = screen right, Y = screen up, Z = away from the viewer.
        return {rolledRight,rolledUp,forward};
    }
};
} // namespace subspace
