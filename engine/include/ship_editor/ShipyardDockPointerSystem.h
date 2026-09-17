#pragma once

// PASS1508: live, renderer-independent mouse manipulation of the authoritative
// SubspaceDockWorkspace. A dock header can be dragged into a floating panel,
// a floating header moves it, its lower-right corner resizes it, and dropping
// a dragged panel on a different dock leaf redocks it. No second layout store.
#include "ui/SubspaceUiFramework.h"
#include "ship_editor/ShipyardPanelCompositorSystem.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace subspace {
class ShipyardDockPointerSystem {
public:
    bool Active() const { return !panel_.empty(); }
    bool Dragged() const { return dragged_; }
    const std::string& Panel() const { return panel_; }
    // Floating UI is opaque to viewport selection even when its body contains
    // no clickable control. Unlike a title drag, this must not capture mouse.
    static bool CoversFloatingPanel(const SubspaceDockWorkspace& w,int width,int height,
                                    float topInset,float x,float y){
        const auto layers=ShipyardPanelCompositorSystem::Snapshot(w,width,height,topInset);
        if(ShipyardPanelCompositorSystem::TopFloatingAt(layers,x,y))return true;
        // Overlay-first Shipyard: docked tools cover the full canvas too.
        // Their empty bodies must never permit selecting the ship through UI.
        if(w.id=="shipyard")for(const auto& layer:layers)
            if(layer.visible&&layer.panelId!="viewport"&&
               ShipyardPanelCompositorSystem::Contains(layer.rect,x,y))return true;
        return false;
    }
    void Cancel() { panel_.clear(); leaf_.clear(); dragged_=false; resizing_=false; accumulatedX_=accumulatedY_=0; }

    bool Begin(const SubspaceDockWorkspace& w,int width,int height,float topInset,float x,float y){
        Cancel();
        if(width<120||height<120||topInset<0||topInset>=height)return false;
        const auto layers=ShipyardPanelCompositorSystem::Snapshot(w,width,height,topInset);
        const auto* topFloating=ShipyardPanelCompositorSystem::TopFloatingAt(layers,x,y);
        // A floating body blocks every dock header underneath it, even if the
        // cursor is not on its own title or resize grip.
        for(int pass=0;pass!=2;++pass){
          if(pass==1&&topFloating)return false;
          for(auto it=layers.rbegin();it!=layers.rend();++it){
            const auto& d=*it;
            if(d.floating!=(pass==0)||!d.visible)continue;
            if(pass==0&&topFloating&&d.panelId!=topFloating->panelId)continue;
            if(d.panelId=="viewport")continue;
            const auto* p=SubspaceDockSystem::FindPanel(w,d.panelId);
            if(!p||!p->visible||!p->floatable)continue;
            const auto& r=d.rect;
            if(x<r.x||y<r.y||x>=r.x+r.width||y>=r.y+r.height)continue;
            const bool resize=d.floating&&p->resizable&&x>=r.x+r.width-18&&y>=r.y+r.height-18;
            const bool title=y<r.y+std::min(d.panelId=="tool_rail"?24.0f:27.0f,r.height)&&x>=r.x+5&&
                x<r.x+r.width-(d.panelId=="tool_rail"?5.0f:103.0f);
            if(!resize&&!title)continue;
            panel_=d.panelId;leaf_=d.leafId.empty()?p->defaultLeafId:d.leafId;
            start_=r;startX_=x;startY_=y;
            resizing_=resize;wasFloating_=d.floating;
            return true;
          }
        }
        return false;
    }

    bool Drag(SubspaceDockWorkspace& w,float dx,float dy,int width,int height,float topInset){
        if(!Active())return false;
        accumulatedX_+=dx;accumulatedY_+=dy;
        if(!dragged_&&accumulatedX_*accumulatedX_+accumulatedY_*accumulatedY_<25.0f)return true;
        if(!dragged_){
            dragged_=true;
            // Hovering/moving a window must never leave it painted below the
            // other float's content. Raise it in the one authoritative stack.
            if(wasFloating_)SubspaceDockSystem::RaiseFloatingPanel(w,panel_);
            if(!wasFloating_){
                auto* p=SubspaceDockSystem::FindPanel(w,panel_);
                if(!p||!p->floatable){Cancel();return false;}
                // Undocking is not a request to shrink the inspector to its
                // narrow split width. Restore its authored usable dimensions.
                const float floatingW=std::clamp(p->preferredWidth,p->minWidth,
                    std::max(p->minWidth,static_cast<float>(width)-12.0f));
                const float floatingH=std::clamp(p->preferredHeight,p->minHeight,
                    std::max(p->minHeight,static_cast<float>(height)-topInset-12.0f));
                start_.x=std::clamp(start_.x,0.0f,std::max(0.0f,static_cast<float>(width)-floatingW));
                start_.y=std::clamp(start_.y,topInset,
                    std::max(topInset,static_cast<float>(height)-floatingH));
                if(!SubspaceDockSystem::FloatPanel(w,panel_,{start_.x,start_.y,floatingW,floatingH})){Cancel();return false;}
                start_.width=floatingW;start_.height=floatingH;
            }
        }
        auto* p=SubspaceDockSystem::FindPanel(w,panel_);
        if(!p){Cancel();return false;}
        for(auto& f:w.floatingPanels)if(f.panelId==panel_){
            auto r=start_;
            if(resizing_){
                const float availableW=std::max(p->minWidth,static_cast<float>(width)-r.x);
                const float availableH=std::max(p->minHeight,static_cast<float>(height)-r.y);
                r.width=std::clamp(start_.width+accumulatedX_,p->minWidth,std::min(p->maxWidth,availableW));
                r.height=std::clamp(start_.height+accumulatedY_,p->minHeight,std::min(p->maxHeight,availableH));
                return SubspaceDockSystem::ResizeFloating(w,panel_,r);
            }
            r.x=std::clamp(start_.x+accumulatedX_,0.0f,std::max(0.0f,static_cast<float>(width)-r.width));
            r.y=std::clamp(start_.y+accumulatedY_,topInset,std::max(topInset,static_cast<float>(height)-r.height));
            f.rect=r;
            return true;
        }
        Cancel();return false;
    }

    bool End(SubspaceDockWorkspace& w,float x,float y,int width,int height,float topInset){
        if(!Active())return false;
        const bool moved=dragged_,resize=resizing_,wasFloating=wasFloating_;
        const std::string panel=panel_,originLeaf=leaf_;
        Cancel();
        if(!moved||resize)return moved;
        // Hit-test the same materialized overlays used for painting and input.
        // A drop over another docked tool joins its tab group. Empty canvas
        // stays floating; four explicit narrow edge targets restore anchors.
        const auto layers=ShipyardPanelCompositorSystem::Snapshot(w,width,height,topInset);
        std::string target;
        for(auto it=layers.rbegin();it!=layers.rend();++it){
            if(it->floating||it->panelId==panel||it->panelId=="viewport")continue;
            if(ShipyardPanelCompositorSystem::Contains(it->rect,x,y)){
                target=it->leafId;break;
            }
        }
        if(target.empty()&&x>=0&&x<width&&y>=topInset&&y<height){
            const float right=static_cast<float>(width),bottom=static_cast<float>(height);
            if(x<36.0f)target="tool_left";
            else if(y>bottom-32.0f)target="bottom";
            else if(x>right-36.0f)target=y<topInset+250.0f?"right_top":"right_bottom";
        }
        if(target.empty()||target=="center"||
           (panel!="tool_rail"&&target=="tool_left")||
           (panel=="tool_rail"&&target!="tool_left")||
           (!wasFloating&&target==originLeaf))return true;
        SubspaceDockSystem::DockPanel(w,panel,target,true);
        return true;
    }

private:
    std::string panel_,leaf_;
    SubspaceUiRect start_{};
    float startX_=0,startY_=0,accumulatedX_=0,accumulatedY_=0;
    bool resizing_=false,wasFloating_=false,dragged_=false;
};
} // namespace subspace
