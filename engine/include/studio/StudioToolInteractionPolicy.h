#pragma once
#include "ship_editor/ShipyardTransformSystem.h"

namespace subspace {
// S15C0A: selecting a target is an explicit Select-tool operation.  Move,
// Rotate and Scale clicks that miss a gizmo cannot silently retarget or begin
// free-dragging an unrelated module.  Outliner selection stays intentional.
struct StudioToolInteractionPolicy {
    static constexpr bool AllowsViewportReselection(ShipyardTransformTool tool) noexcept {
        return tool == ShipyardTransformTool::Select;
    }
    static constexpr bool AllowsGizmoGesture(ShipyardTransformTool tool) noexcept {
        return tool != ShipyardTransformTool::Select;
    }
};
} // namespace subspace
