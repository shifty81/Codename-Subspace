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
        return ShipyardPanelCompositorSystem::TopFloatingAt(layers,x,y)!=nullptr;
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
            if(d.panelId=="viewport"||d.panelId=="tool_rail")continue;
            const auto* p=SubspaceDockSystem::FindPanel(w,d.panelId);
            if(!p||!p->visible||!p->floatable)continue;
            const auto& r=d.rect;
            if(x<r.x||y<r.y||x>=r.x+r.width||y>=r.y+r.height)continue;
            const bool resize=d.floating&&p->resizable&&x>=r.x+r.width-18&&y>=r.y+r.height-18;
            const bool title=y<r.y+std::min(27.0f,r.height)&&x>=r.x+5&&x<r.x+r.width-103;
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
        const std::string target=LeafAt(w,w.rootNodeId,
            {0,topInset,static_cast<float>(width),static_cast<float>(height)-topInset},x,y);
        // A fixed shell rail and permanent 3D View are never drop targets.
        // The bottom shelf is a sibling inside the content subtree (not a
        // child of the tool rail), and same-leaf undocking stays floating.
        if(target.empty()||target=="center"||target=="tool_left"||(!wasFloating&&target==originLeaf))return true;
        SubspaceDockSystem::DockPanel(w,panel,target,true);
        return true;
    }

private:
    static std::string LeafAt(const SubspaceDockWorkspace& w,const std::string& id,
                              SubspaceUiRect r,float x,float y){
        if(x<r.x||y<r.y||x>=r.x+r.width||y>=r.y+r.height)return {};
        const auto* n=SubspaceDockSystem::FindNode(w,id);
        if(!n||n->collapsed)return {};
        if(!n->split)return id;
        const float ratio=std::clamp(n->ratio,.08f,.92f);
        auto a=r,b=r;
        if(n->axis==SubspaceDockSplitAxis::Horizontal){a.width=r.width*ratio;b.x=r.x+a.width;b.width=r.width-a.width;}
        else {a.height=r.height*ratio;b.y=r.y+a.height;b.height=r.height-a.height;}
        auto hit=LeafAt(w,n->firstChildId,a,x,y);
        return hit.empty()?LeafAt(w,n->secondChildId,b,x,y):hit;
    }
    std::string panel_,leaf_;
    SubspaceUiRect start_{};
    float startX_=0,startY_=0,accumulatedX_=0,accumulatedY_=0;
    bool resizing_=false,wasFloating_=false,dragged_=false;
};
} // namespace subspace
