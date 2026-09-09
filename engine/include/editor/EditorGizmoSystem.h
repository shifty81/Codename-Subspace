#pragma once
#include "core/Math.h"
namespace subspace {struct EditorScreenDirection{float x=0,y=-1;bool valid=false;};class EditorGizmoSystem{public:static EditorScreenDirection ProjectPlanarDirection(const Vector3&,float cameraYawDegrees,float verticalCompression=.78f);};}
