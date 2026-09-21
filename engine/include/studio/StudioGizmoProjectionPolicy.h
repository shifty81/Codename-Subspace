#pragma once
#include "studio/StudioGizmoMath.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace subspace {
// A gizmo handle is an on-screen manipulation affordance, not a 3D mesh.
// Physical XYZ selection is retained even if the projected direction must
// move on screen to remain visible, distinct, and mouse-pickable.
struct StudioGizmoProjectionPolicy {
    struct Bounds { float left=0,top=0,right=0,bottom=0; };
    static constexpr float kMinimumProjectedPixels = 1.8f;
    static constexpr float kHandlePixels = 68.0f;
    static constexpr float kMinimumHandlePixels = 44.0f;
    static constexpr float kMinimumTipSpacingPixels = 44.0f;
    static constexpr float kMaximumDirectionDot = 0.78f;
    static bool UsesScreenSpaceFallback(StudioPoint delta) noexcept {
        return !std::isfinite(delta.x) || !std::isfinite(delta.y) ||
               StudioGizmoMath::Length(delta) < kMinimumProjectedPixels;
    }
    // WorldToScreen's `visible` flag also rejects off-screen probe points.
    // An off-screen point IN FRONT of the near plane has a useful projection;
    // do not drop its axis just because the object is large or zoomed in.
    static bool ProbeUsable(float depth, StudioPoint projected) noexcept {
        return std::isfinite(depth) && depth > 0.08f &&
               std::isfinite(projected.x) && std::isfinite(projected.y);
    }
    static StudioPoint Direction(StudioAxis axis, StudioPoint delta) noexcept {
        if (!UsesScreenSpaceFallback(delta)) return StudioGizmoMath::Unit(delta);
        switch (axis) {
            case StudioAxis::X: return {1.0f, 0.0f};
            case StudioAxis::Y: return {-0.70710678f, -0.70710678f};
            case StudioAxis::Z: return {0.0f, 1.0f};
            default: return {0.0f, 0.0f};
        }
    }
    static float AvailableLength(StudioPoint center,StudioPoint unit,Bounds b) noexcept {
        float length=kHandlePixels;
        if(unit.x>0.0001f)length=std::min(length,(b.right-center.x)/unit.x);
        else if(unit.x<-0.0001f)length=std::min(length,(b.left-center.x)/unit.x);
        if(unit.y>0.0001f)length=std::min(length,(b.bottom-center.y)/unit.y);
        else if(unit.y<-0.0001f)length=std::min(length,(b.top-center.y)/unit.y);
        return std::isfinite(length)?length:0.0f;
    }
    static StudioPoint Rotate45(StudioPoint v,int eighthTurns) noexcept {
        constexpr float c=0.7071067811865475f;
        switch((eighthTurns+8)%8){
            case 0:return v;
            case 1:return {(v.x-v.y)*c,(v.x+v.y)*c};
            case 2:return {-v.y,v.x};
            case 3:return {-(v.x+v.y)*c,(v.x-v.y)*c};
            case 4:return {-v.x,-v.y};
            case 5:return {(v.y-v.x)*c,-(v.x+v.y)*c};
            case 6:return {v.y,-v.x};
            default:return {(v.x+v.y)*c,(v.y-v.x)*c};
        }
    }
    // One deterministic layout for BOTH render and hit-test snapshots.
    // A non-degenerate projection can still overlap another axis; R3 only
    // handled the special case where an individual projection was <1.8 px.
    // Floating docks are part of the viewport: do not discard a usable axis
    // after the projection pass. Reflow *the same* snapshot used for painting
    // and picking so the visible tip remains the actual hit target.
    template<class Occluded>
    static std::array<StudioAxisHandle,3> ReflowForOcclusion(
        const std::array<StudioAxisHandle,3>& projected,Bounds bounds,Occluded occluded) noexcept {
        std::array<StudioAxisHandle,3> result{};
        const StudioPoint center=projected[0].center;
        if(!std::isfinite(center.x)||!std::isfinite(center.y)||occluded(center.x,center.y))return result;
        constexpr std::array<int,8> offsets{{0,1,-1,2,-2,3,-3,4}};
        for(std::size_t i=0;i<3;++i){
            auto& handle=result[i];handle.axis=static_cast<StudioAxis>(i);handle.center=center;
            if(!projected[i].valid)continue;
            const auto base=StudioGizmoMath::Unit(StudioGizmoMath::Delta(projected[i].tip,center));
            for(const int offset:offsets){
                const auto direction=Rotate45(base,offset);
                const float length=AvailableLength(center,direction,bounds);
                if(length<kMinimumHandlePixels)continue;
                const StudioPoint tip{center.x+direction.x*length,center.y+direction.y*length};
                bool blocked=false;
                // Probe the entire visible shaft and the marker's footprint,
                // not just its centre. Picking must never pass through a dock.
                for(int step=1;step<=12&&!blocked;++step){
                    const float t=static_cast<float>(step)/12.0f;
                    blocked=occluded(center.x+(tip.x-center.x)*t,
                                     center.y+(tip.y-center.y)*t);
                }
                if(!blocked){
                    for(const auto p:std::array<StudioPoint,4>{{
                        {tip.x-7,tip.y},{tip.x+7,tip.y},
                        {tip.x,tip.y-7},{tip.x,tip.y+7}}}){
                        if(occluded(p.x,p.y)){blocked=true;break;}
                    }
                }
                if(blocked)continue;
                bool distinct=true;
                for(std::size_t j=0;j<i;++j){
                    if(!result[j].valid)continue;
                    const auto previous=StudioGizmoMath::Unit(
                        StudioGizmoMath::Delta(result[j].tip,center));
                    if(StudioGizmoMath::Dot(previous,direction)>kMaximumDirectionDot ||
                       StudioGizmoMath::Length(StudioGizmoMath::Delta(result[j].tip,tip))<kMinimumTipSpacingPixels){
                        distinct=false;break;
                    }
                }
                if(!distinct)continue;
                handle.tip=tip;handle.valid=true;break;
            }
        }
        return result;
    }
    static std::array<StudioAxisHandle,3> BuildHandles(
        StudioPoint center,const std::array<StudioPoint,3>& preferred,Bounds bounds) noexcept {
        std::array<StudioAxisHandle,3> handles{};
        if(!std::isfinite(center.x)||!std::isfinite(center.y)||
           center.x<bounds.left||center.x>bounds.right||
           center.y<bounds.top||center.y>bounds.bottom)return handles;
        constexpr std::array<int,8> offsets{{0,1,-1,2,-2,3,-3,4}};
        for(std::size_t i=0;i<handles.size();++i){
            auto& handle=handles[i];handle.axis=static_cast<StudioAxis>(i);handle.center=center;
            const StudioPoint base=StudioGizmoMath::Unit(preferred[i]);
            if(StudioGizmoMath::Length(base)<0.5f)continue;
            for(const int offset:offsets){
                const StudioPoint direction=Rotate45(base,offset);
                const float length=AvailableLength(center,direction,bounds);
                if(length<kMinimumHandlePixels)continue;
                const StudioPoint tip{center.x+direction.x*length,center.y+direction.y*length};
                bool distinct=true;
                for(std::size_t j=0;j<i;++j){
                    if(!handles[j].valid)continue;
                    const auto previous=StudioGizmoMath::Unit(
                        StudioGizmoMath::Delta(handles[j].tip,center));
                    if(StudioGizmoMath::Dot(previous,direction)>kMaximumDirectionDot ||
                       StudioGizmoMath::Length(StudioGizmoMath::Delta(handles[j].tip,tip))<kMinimumTipSpacingPixels){
                        distinct=false;break;
                    }
                }
                if(!distinct)continue;
                handle.tip=tip;handle.valid=true;break;
            }
        }
        // Extremely corner-clipped projections can leave no viable third
        // direction after the preferred-direction pass. Replace the WHOLE
        // trio with a stable inward fan rather than returning a partial tool.
        if(!handles[0].valid||!handles[1].valid||!handles[2].valid){
            const float sx=(bounds.right-center.x >= center.x-bounds.left)?1.0f:-1.0f;
            const float sy=(bounds.bottom-center.y >= center.y-bounds.top)?1.0f:-1.0f;
            constexpr float diagonal=0.7071067811865475f;
            const std::array<StudioPoint,3> fan{{{sx,0},{sx*diagonal,sy*diagonal},{0,sy}}};
            for(std::size_t i=0;i<3;++i){
                handles[i].axis=static_cast<StudioAxis>(i);handles[i].center=center;
                const float length=AvailableLength(center,fan[i],bounds);
                handles[i].valid=length>=kMinimumHandlePixels;
                if(handles[i].valid)handles[i].tip={center.x+fan[i].x*length,center.y+fan[i].y*length};
            }
        }
        return handles;
    }
};
} // namespace subspace
