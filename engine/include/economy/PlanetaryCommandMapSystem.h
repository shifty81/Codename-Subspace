#pragma once

#include "economy/PlanetaryIndustrySystem.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace subspace {

enum class PlanetCommandOverlay {
    Territory = 0,
    Resources,
    Hazards,
    Logistics,
    Landing,
    Development
};

struct PlanetCommandCamera {
    float yawDegrees = 0.0f;
    float pitchDegrees = 18.0f;
    float zoom = 1.0f;
};

struct PlanetCommandMapLayout {
    float left = 64.0f;
    float top = 62.0f;
    float right = 1536.0f;
    float bottom = 842.0f;
    float infoLeft = 1210.0f;
    float centerX = 640.0f;
    float centerY = 450.0f;
    float globeRadius = 270.0f;
};

struct PlanetCommandProjectedHex {
    HexCoord coord{};
    float x = 0.0f;
    float y = 0.0f;
    float depth = 0.0f;
    float radius = 0.0f;
    bool frontFacing = false;
    bool frontier = false;
};

class PlanetaryCommandMapSystem {
public:
    static constexpr float Pi() noexcept { return 3.14159265358979323846f; }

    static bool Same(const HexCoord& a,const HexCoord& b) noexcept {
        return a.q==b.q && a.r==b.r;
    }

    static int HexRadius(const HexCoord& c) noexcept {
        return std::max({std::abs(c.q),std::abs(c.r),std::abs(-c.q-c.r)});
    }

    static int GridRadius(const PlanetaryIndustryState& state) noexcept {
        int radius=1;
        for(const auto& kv:state.hexes)radius=std::max(radius,HexRadius(kv.first));
        return radius;
    }

    static bool IsFrontier(const PlanetaryIndustryState& state,const HexCoord& c) noexcept {
        const auto it=state.hexes.find(c);
        if(it==state.hexes.end()||it->second.claimed)return false;
        static constexpr HexCoord dirs[6]={{1,0},{1,-1},{0,-1},{-1,0},{-1,1},{0,1}};
        for(const auto& d:dirs){
            const HexCoord n{c.q+d.q,c.r+d.r};
            const auto ni=state.hexes.find(n);
            if(ni!=state.hexes.end()&&ni->second.claimed)return true;
        }
        return false;
    }

    static bool IsSelectable(const PlanetaryIndustryState& state,const HexCoord& c) noexcept {
        const auto it=state.hexes.find(c);
        if(it==state.hexes.end())return false;
        const auto& h=it->second;
        return h.surveyed||h.claimed||h.developed||IsFrontier(state,c);
    }

    static PlanetCommandMapLayout Layout(int width,int height) noexcept {
        PlanetCommandMapLayout l;
        l.left=64.0f;
        l.top=62.0f;
        l.right=std::max(l.left+760.0f,static_cast<float>(width)-64.0f);
        l.bottom=std::max(l.top+560.0f,static_cast<float>(height)-58.0f);
        const float totalW=l.right-l.left;
        const float infoW=std::clamp(totalW*.255f,320.0f,430.0f);
        l.infoLeft=l.right-infoW;
        const float plotW=l.infoLeft-l.left;
        const float plotH=l.bottom-l.top;
        l.centerX=l.left+plotW*.50f;
        l.centerY=l.top+plotH*.52f;
        l.globeRadius=std::max(150.0f,std::min(plotW*.34f,plotH*.38f));
        return l;
    }

    static PlanetCommandProjectedHex Project(
        const PlanetaryIndustryState& state,const HexCoord& coord,
        const PlanetCommandCamera& camera,const PlanetCommandMapLayout& layout) noexcept {
        PlanetCommandProjectedHex out;
        out.coord=coord;
        out.frontier=IsFrontier(state,coord);

        const float grid=static_cast<float>(std::max(1,GridRadius(state)));
        const float fx=(static_cast<float>(coord.q)+static_cast<float>(coord.r)*.5f)/grid;
        const float fy=(static_cast<float>(coord.r)*.8660254038f)/grid;
        const float lon=fx*1.28f;
        const float lat=std::clamp(fy*1.04f,-1.15f,1.15f);

        const float cl=std::cos(lat);
        float x=cl*std::sin(lon);
        float y=std::sin(lat);
        float z=cl*std::cos(lon);

        const float yaw=camera.yawDegrees*Pi()/180.0f;
        const float cy=std::cos(yaw),sy=std::sin(yaw);
        const float x1=x*cy-z*sy,z1=x*sy+z*cy;

        const float pitch=std::clamp(camera.pitchDegrees,-72.0f,72.0f)*Pi()/180.0f;
        const float cp=std::cos(pitch),sp=std::sin(pitch);
        const float y1=y*cp-z1*sp,z2=y*sp+z1*cp;

        const float zoom=std::clamp(camera.zoom,.65f,1.35f);
        const float sphereRadius=layout.globeRadius*zoom;
        out.x=layout.centerX+x1*sphereRadius;
        out.y=layout.centerY-y1*sphereRadius;
        out.depth=z2;
        out.frontFacing=z2>0.045f;
        const float base=sphereRadius/(grid*4.35f+3.0f);
        out.radius=std::max(6.0f,base*(.60f+.40f*std::clamp(z2,0.0f,1.0f)));
        return out;
    }

    static std::vector<PlanetCommandProjectedHex> ProjectSelectable(
        const PlanetaryIndustryState& state,const PlanetCommandCamera& camera,
        const PlanetCommandMapLayout& layout) {
        std::vector<PlanetCommandProjectedHex> out;
        out.reserve(state.hexes.size());
        for(const auto& kv:state.hexes){
            if(!IsSelectable(state,kv.first))continue;
            auto p=Project(state,kv.first,camera,layout);
            if(p.frontFacing)out.push_back(p);
        }
        std::sort(out.begin(),out.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
        return out;
    }

    static bool HitTest(const PlanetaryIndustryState& state,const PlanetCommandCamera& camera,
                        const PlanetCommandMapLayout& layout,float screenX,float screenY,
                        HexCoord& hit) {
        const auto projected=ProjectSelectable(state,camera,layout);
        float best=1.0e30f;
        bool found=false;
        for(const auto& p:projected){
            const float dx=screenX-p.x,dy=screenY-p.y;
            const float d2=dx*dx+dy*dy;
            const float r=p.radius*1.25f;
            if(d2<=r*r&&d2<best){best=d2;hit=p.coord;found=true;}
        }
        return found;
    }

    static bool StepSelection(const PlanetaryIndustryState& state,const PlanetCommandCamera& camera,
                              const PlanetCommandMapLayout& layout,HexCoord& selected,int direction) {
        auto projected=ProjectSelectable(state,camera,layout);
        if(projected.empty()||direction==0)return false;
        std::sort(projected.begin(),projected.end(),[](const auto& a,const auto& b){
            if(std::fabs(a.y-b.y)>2.0f)return a.y<b.y;
            return a.x<b.x;
        });
        std::size_t index=0;
        for(std::size_t i=0;i<projected.size();++i){
            if(Same(projected[i].coord,selected)){index=i;break;}
        }
        if(direction>0)index=(index+1)%projected.size();
        else index=(index+projected.size()-1)%projected.size();
        selected=projected[index].coord;
        return true;
    }

    static const char* OverlayName(PlanetCommandOverlay overlay) noexcept {
        switch(overlay){
            case PlanetCommandOverlay::Resources:return "RESOURCES";
            case PlanetCommandOverlay::Hazards:return "HAZARDS";
            case PlanetCommandOverlay::Logistics:return "LOGISTICS";
            case PlanetCommandOverlay::Landing:return "LANDING";
            case PlanetCommandOverlay::Development:return "DEVELOPMENT";
            default:return "TERRITORY";
        }
    }

    static void CycleOverlay(PlanetCommandOverlay& overlay,int direction=1) noexcept {
        constexpr int count=6;
        int value=static_cast<int>(overlay);
        value=(value+(direction>=0?1:-1)+count)%count;
        overlay=static_cast<PlanetCommandOverlay>(value);
    }
};

} // namespace subspace
