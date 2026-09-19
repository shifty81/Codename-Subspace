#include "studio/StudioToolInteractionPolicy.h"
#include <cassert>
#include <iostream>
using namespace subspace;
int main(){
    static_assert(StudioToolInteractionPolicy::AllowsViewportReselection(ShipyardTransformTool::Select));
    static_assert(!StudioToolInteractionPolicy::AllowsViewportReselection(ShipyardTransformTool::Move));
    static_assert(!StudioToolInteractionPolicy::AllowsViewportReselection(ShipyardTransformTool::Rotate));
    static_assert(!StudioToolInteractionPolicy::AllowsViewportReselection(ShipyardTransformTool::Scale));
    static_assert(!StudioToolInteractionPolicy::AllowsGizmoGesture(ShipyardTransformTool::Select));
    static_assert(StudioToolInteractionPolicy::AllowsGizmoGesture(ShipyardTransformTool::Move));
    static_assert(StudioToolInteractionPolicy::AllowsGizmoGesture(ShipyardTransformTool::Rotate));
    static_assert(StudioToolInteractionPolicy::AllowsGizmoGesture(ShipyardTransformTool::Scale));
    std::cout << "S15C0A policy smoke PASS\n";
}
