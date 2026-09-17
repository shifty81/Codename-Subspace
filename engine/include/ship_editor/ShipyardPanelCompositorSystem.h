#pragma once

// One ordered dock snapshot for Shipyard rendering, hit-testing and pointer
// occlusion. The dock workspace remains the sole state authority. No GUI copy
// or per-renderer floating-panel store is permitted.
#include "ui/SubspaceUiFramework.h"

#include <algorithm>
#include <string>
#include <vector>

namespace subspace {
class ShipyardPanelCompositorSystem {
public:
    using Layers = std::vector<SubspaceDockLayout>;

    static Layers Snapshot(const SubspaceDockWorkspace& workspace,int width,int height,float topInset){
        // Materialize puts dock leaves first and floating windows last, in
        // their back-to-front stacking order. Preserve it without sorting.
        return SubspaceDockSystem::Materialize(workspace,width,height,topInset);
    }
    static bool Contains(const SubspaceUiRect& rect,float x,float y){
        return rect.width>0.0f&&rect.height>0.0f&&
            x>=rect.x&&y>=rect.y&&x<rect.x+rect.width&&y<rect.y+rect.height;
    }
    static const SubspaceDockLayout* TopFloatingAt(const Layers& layers,float x,float y){
        for(auto it=layers.rbegin();it!=layers.rend();++it)
            if(it->visible&&it->floating&&Contains(it->rect,x,y))return &*it;
        return nullptr;
    }
    static const SubspaceDockLayout* Panel(const Layers& layers,const std::string& panelId){
        for(const auto& layer:layers)if(layer.visible&&layer.panelId==panelId)return &layer;
        return nullptr;
    }
    static bool IsFloating(const SubspaceDockWorkspace& workspace,const std::string& panelId){
        return std::any_of(workspace.floatingPanels.begin(),workspace.floatingPanels.end(),
            [&](const auto& floating){return floating.panelId==panelId;});
    }
};
} // namespace subspace
