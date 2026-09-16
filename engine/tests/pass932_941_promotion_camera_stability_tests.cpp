#include "editor/ConstructionEditorCameraSystem.h"

#include <cmath>
#include <iostream>

using namespace subspace;

namespace {
int passed = 0;
int failed = 0;

void Test(const char* name, bool ok) {
    if (ok) ++passed;
    else { ++failed; std::cerr << "FAIL: " << name << "\n"; }
}

bool Near(float a, float b, float eps = 1.0e-4f) { return std::fabs(a-b) <= eps; }
bool NearVec(const Vector3& a, const Vector3& b, float eps = 1.0e-4f) {
    return Near(a.x,b.x,eps) && Near(a.y,b.y,eps) && Near(a.z,b.z,eps);
}
Vector3 Normalize(Vector3 v) {
    const float len=v.length();
    return len>1.0e-6f?v*(1.0f/len):Vector3{};
}
}

int main() {
    ConstructionEditorCameraState camera;
    ConstructionEditorCameraSystem::Reset(camera,{4.0f,8.0f,2.0f},10.0f);
    Test("reset targets assembly center", NearVec(ConstructionEditorCameraSystem::Target(camera),camera.assemblyCenter));

    const Vector3 targetBeforePan=ConstructionEditorCameraSystem::Target(camera);
    const Vector3 eyeBeforePan=camera.eye;
    ConstructionEditorCameraSystem::TruckPedestal(camera,3.25f,-1.5f);
    Test("truck moves editor eye", (camera.eye-eyeBeforePan).length()>0.1f);
    const Vector3 targetAfterPan=ConstructionEditorCameraSystem::Target(camera);
    const Vector3 panDelta=camera.eye-eyeBeforePan;
    Test("truck pans editor target with eye", NearVec(targetAfterPan,targetBeforePan+panDelta));
    Test("truck preserves eye-target ray", NearVec(camera.eye-targetAfterPan,eyeBeforePan-targetBeforePan));

    const Vector3 targetBeforeZoom=ConstructionEditorCameraSystem::Target(camera);
    const Vector3 rayBeforeZoom=Normalize(camera.eye-targetBeforeZoom);
    const float distanceBeforeZoom=(camera.eye-targetBeforeZoom).length();
    ConstructionEditorCameraSystem::Dolly(camera,2.0f);
    const Vector3 rayAfterZoom=Normalize(camera.eye-ConstructionEditorCameraSystem::Target(camera));
    Test("dolly preserves editor target", NearVec(ConstructionEditorCameraSystem::Target(camera),targetBeforeZoom));
    Test("dolly preserves current camera placement ray", NearVec(rayBeforeZoom,rayAfterZoom));
    Test("dolly changes only distance along current placement ray", (camera.eye-targetBeforeZoom).length()<distanceBeforeZoom);

    const Vector3 targetBeforeOrbit=ConstructionEditorCameraSystem::Target(camera);
    ConstructionEditorCameraSystem::Orbit(camera,12.0f,-4.0f);
    Test("orbit after zoom keeps assembly target", NearVec(ConstructionEditorCameraSystem::Target(camera),targetBeforeOrbit));

    ConstructionEditorCameraSystem::BeginFreeFly(camera);
    ConstructionEditorCameraSystem::MoveFree(camera,1.0f,0.5f,0.0f,0.25f);
    ConstructionEditorCameraSystem::EndFreeFly(camera);
    Test("free-fly release retains established assembly-center contract", NearVec(ConstructionEditorCameraSystem::Target(camera),camera.assemblyCenter));

    std::cout << "Pass932-941 PCC/Camera Stability: " << passed << " passed / " << failed << " failed\n";
    return failed==0?0:1;
}
