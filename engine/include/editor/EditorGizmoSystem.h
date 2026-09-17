#pragma once
#include "core/Math.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "rendering/StrategicCamera.h"
#include "ship_editor/ShipyardTransformSystem.h"
#include <array>

namespace subspace {
struct EditorScreenDirection {float x=0,y=-1;bool valid=false;};
enum class EditorGizmoAxis {None=-1,X=0,Y=1,Z=2};
struct EditorGizmoHandle {
    EditorGizmoAxis axis=EditorGizmoAxis::None;
    Vector3 assemblyDirection{}; // assembly-space displacement; scale/rotation use axis identity
    float centerX=0,centerY=0,tipX=0,tipY=0;
    bool valid=false;
};
struct EditorGizmoLayout {
    std::array<EditorGizmoHandle,3> handles{};
    bool visible=false;
};
class EditorGizmoSystem {
public:
    static EditorScreenDirection ProjectPlanarDirection(const Vector3&,float cameraYawDegrees,float verticalCompression=.78f);
    // One projection/picking authority for the native renderer and both shipyard input paths.
    // The module pivot follows the same render root transform, role-specific scale and forward yaw.
    static EditorGizmoLayout BuildModule(const VisualModulePlacement& module,
        const ProceduralShipVisualRecipe& recipe,const StrategicCamera& camera,
        float viewportWidth,float viewportHeight,const Vector3& shipWorldPosition,
        float runtimeShipYawRadians,ShipyardTransformSpace space,ShipyardTransformTool tool);
    static EditorGizmoAxis Pick(const EditorGizmoLayout&,float x,float y,float thresholdPixels=9.0f);
    static const EditorGizmoHandle* Handle(const EditorGizmoLayout&,EditorGizmoAxis);
    static float DragPixels(const EditorGizmoHandle&,float deltaX,float deltaY);
};
} // namespace subspace
