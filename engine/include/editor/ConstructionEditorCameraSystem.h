#pragma once

#include "core/Math.h"

namespace subspace {

enum class ConstructionCameraMode { CenteredInspect = 0, FreeFly };

struct ConstructionEditorCameraState {
    ConstructionCameraMode mode = ConstructionCameraMode::CenteredInspect;
    Vector3 assemblyCenter{};
    Vector3 eye{8.0f,-14.0f,8.0f};
    Vector3 forward{-0.35f,0.82f,-0.45f};
    Vector3 up{0.0f,0.0f,1.0f};
    float rollDegrees = 0.0f;
    float moveSpeed = 7.0f;
    float orbitDistance = 18.0f;
    float yawDegrees = 18.0f;
    float pitchDegrees = 28.0f;
    bool centerLock = true;
};

/// 6DOF construction/editor camera authority. Centered inspection keeps the
/// assembly on-screen while moving the camera; holding Alt switches input to
/// a free-fly camera without moving the authored ship/station/weapon itself.
class ConstructionEditorCameraSystem {
public:
    static void Reset(ConstructionEditorCameraState& state,const Vector3& center,float radius=6.0f);
    static void SetAssemblyCenter(ConstructionEditorCameraState& state,const Vector3& center,bool preserveEye=true);
    static void Orbit(ConstructionEditorCameraState& state,float deltaYaw,float deltaPitch);
    static void TruckPedestal(ConstructionEditorCameraState& state,float deltaRight,float deltaUp);
    static void BeginFreeFly(ConstructionEditorCameraState& state);
    static void EndFreeFly(ConstructionEditorCameraState& state);
    static void Look(ConstructionEditorCameraState& state,float deltaYaw,float deltaPitch);
    static void Roll(ConstructionEditorCameraState& state,float deltaDegrees);
    static void MoveFree(ConstructionEditorCameraState& state,float forward,float right,float up,float deltaSeconds);
    static void AdjustSpeed(ConstructionEditorCameraState& state,float wheelSteps);
    static void Dolly(ConstructionEditorCameraState& state,float wheelSteps);
    static Vector3 Target(const ConstructionEditorCameraState& state);
};

} // namespace subspace
