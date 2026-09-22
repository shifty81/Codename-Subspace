#pragma once
#include "ship_editor/ShipyardPanelCompositorSystem.h"
#include "studio/StudioGizmoMath.h"
#include <cmath>

namespace subspace {
// Read-only Studio interaction rules. Layout remains owned by the dock system,
// and transforms remain owned by the existing builder transaction.
struct StudioGuiInteractionPolicy {
    static constexpr float kPointerActivationPixels=3.0f;
    static bool DragActivated(StudioPoint distanceFromPress) noexcept {
        return std::isfinite(distanceFromPress.x)&&std::isfinite(distanceFromPress.y)&&
            StudioGizmoMath::Dot(distanceFromPress,distanceFromPress)>=
                kPointerActivationPixels*kPointerActivationPixels;
    }
    static bool ClearOverlayArea(const ShipyardPanelCompositorSystem::Layers& layers,
                                 SubspaceUiRect area) noexcept {
        if(area.width<=0||area.height<=0)return false;
        for(const auto& layer:layers){
            // Docked bodies cannot overlap a correctly sized viewport; floating
            // windows can. Test the whole HUD rect, not nine sampled pixels.
            if(layer.visible&&layer.floating&&
               ShipyardPanelCompositorSystem::Intersects(layer.rect,area))return false;
        }
        return true;
    }
};
} // namespace subspace
