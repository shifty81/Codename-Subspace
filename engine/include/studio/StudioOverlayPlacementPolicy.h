#pragma once

#include "ship_editor/ShipyardPanelCompositorSystem.h"
#include <array>
#include <algorithm>
#include <cmath>

namespace subspace {

struct StudioOverlayPlacement {
    SubspaceUiRect rect{};
    bool visible=false;
    int slot=-1; // 0 TL, 1 TR, 2 BL, 3 BR
};

// Keeps the Studio-only measurement HUD inside the actual viewport while
// respecting floating dock bodies. This is presentation policy only: it does
// not own dock state, transforms, picking, or document state.
class StudioOverlayPlacementPolicy {
    static bool Intersects(const SubspaceUiRect& a,const SubspaceUiRect& b) noexcept {
        return a.width>0.0f&&a.height>0.0f&&b.width>0.0f&&b.height>0.0f&&
            a.x<b.x+b.width&&b.x<a.x+a.width&&
            a.y<b.y+b.height&&b.y<a.y+a.height;
    }
public:
    static constexpr float kHudWidth=435.0f;
    static constexpr float kHudHeight=130.0f;
    static constexpr float kPadding=9.0f;

    static StudioOverlayPlacement Choose(const ShipyardPanelCompositorSystem::Layers& layers,
                                         float viewportLeft,float viewportTop,
                                         float viewportRight,float viewportBottom) noexcept {
        StudioOverlayPlacement out;
        if(!std::isfinite(viewportLeft)||!std::isfinite(viewportTop)||
           !std::isfinite(viewportRight)||!std::isfinite(viewportBottom))return out;
        const float width=viewportRight-viewportLeft;
        const float height=viewportBottom-viewportTop;
        if(width<kHudWidth+2*kPadding||height<kHudHeight+2*kPadding)return out;

        const float left=viewportLeft+kPadding;
        const float right=viewportRight-kPadding-kHudWidth;
        const float top=viewportTop+kPadding;
        const float bottom=viewportBottom-kPadding-kHudHeight;
        const std::array<SubspaceUiRect,4> candidates{{
            {left,top,kHudWidth,kHudHeight},
            {right,top,kHudWidth,kHudHeight},
            {left,bottom,kHudWidth,kHudHeight},
            {right,bottom,kHudWidth,kHudHeight}
        }};

        for(std::size_t i=0;i<candidates.size();++i){
            bool blocked=false;
            for(const auto& layer:layers){
                if(layer.visible&&layer.floating&&
                   Intersects(layer.rect,candidates[i])){
                    blocked=true;break;
                }
            }
            if(!blocked){out.rect=candidates[i];out.visible=true;out.slot=static_cast<int>(i);return out;}
        }
        return out; // fail closed: hiding is preferable to painting through a window
    }
};

} // namespace subspace
