#pragma once

#include "rendering/ProceduralVisualVariantSystem.h"
#include "core/Math.h"

#include <cstddef>

namespace subspace {

enum class ShipyardTransformTool { Select, Move, Rotate, Scale };
enum class ShipyardTransformSpace { View, Ship, Local };

struct ShipyardTransformTransaction {
    bool active = false;
    std::size_t moduleIndex = 0;
    ShipyardTransformTool tool = ShipyardTransformTool::Select;
    ShipyardTransformSpace space = ShipyardTransformSpace::View;
    VisualModulePlacement before{};
    VisualModulePlacement working{};
    bool snap = true;
    float translationSnap = 0.10f;
    float rotationSnapDegrees = 15.0f;
    float scaleSnap = 0.05f;
};

class ShipyardTransformSystem {
public:
    static bool Begin(ShipyardTransformTransaction& tx,std::size_t moduleIndex,
                      const VisualModulePlacement& placement,ShipyardTransformTool tool,
                      ShipyardTransformSpace space=ShipyardTransformSpace::View);
    static void Translate(ShipyardTransformTransaction& tx,const Vector3& delta,bool fine=false);
    static void Rotate(ShipyardTransformTransaction& tx,const Vector3& deltaDegrees,bool fine=false);
    static void Scale(ShipyardTransformTransaction& tx,const Vector3& deltaScale,bool fine=false);
    static VisualModulePlacement Commit(ShipyardTransformTransaction& tx);
    static VisualModulePlacement Cancel(ShipyardTransformTransaction& tx);
};

} // namespace subspace
