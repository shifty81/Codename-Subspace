#include "interior/ShipInteriorDerivedShellSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace subspace {
namespace {
constexpr float kEps = 0.0001f;
struct Rect { float u0=0,u1=0,v0=0,v1=0; };
float Component(const Vector3& p,int axis){return axis==0?p.x:(axis==1?p.y:p.z);}
void Put(Vector3& p,int axis,float value){if(axis==0)p.x=value;else if(axis==1)p.y=value;else p.z=value;}
int UAxis(int axis){return axis==0?1:0;}
int VAxis(int axis){return axis==2?1:2;}
float Lo(const InteriorCarvedVolume& v,int axis){return Component(v.center,axis)-Component(v.halfExtents,axis);}
float Hi(const InteriorCarvedVolume& v,int axis){return Component(v.center,axis)+Component(v.halfExtents,axis);}
Rect Face(const InteriorCarvedVolume& v,int axis){const int u=UAxis(axis),w=VAxis(axis);return {Lo(v,u),Hi(v,u),Lo(v,w),Hi(v,w)};}
bool Valid(const Rect& r){return r.u1-r.u0>kEps&&r.v1-r.v0>kEps;}
void Add(std::vector<Rect>& list,Rect r){if(Valid(r))list.push_back(r);}
// Exact axis-aligned polygon difference: split a quad into at most four quads.
// This simultaneously removes visual faces and their collision representation.
void Subtract(std::vector<Rect>& pieces,const Rect& cutter){
    std::vector<Rect> next;
    for(const auto& r:pieces){
        const float u0=std::max(r.u0,cutter.u0),u1=std::min(r.u1,cutter.u1);
        const float v0=std::max(r.v0,cutter.v0),v1=std::min(r.v1,cutter.v1);
        if(u1-u0<=kEps||v1-v0<=kEps){next.push_back(r);continue;}
        Add(next,{r.u0,u0,r.v0,r.v1});Add(next,{u1,r.u1,r.v0,r.v1});
        Add(next,{u0,u1,r.v0,v0});Add(next,{u0,u1,v1,r.v1});
    }
    pieces=std::move(next);
}
Vector3 Vertex(int axis,float plane,float u,float v){Vector3 p{};Put(p,axis,plane);Put(p,UAxis(axis),u);Put(p,VAxis(axis),v);return p;}
float CrossNormal(const InteriorShellSurface& s){
    const auto& a=s.corners[0];const auto& b=s.corners[1];const auto& c=s.corners[2];
    const Vector3 x{b.x-a.x,b.y-a.y,b.z-a.z},y{c.x-a.x,c.y-a.y,c.z-a.z};
    return s.axis==0?x.y*y.z-x.z*y.y:(s.axis==1?x.z*y.x-x.x*y.z:x.x*y.y-x.y*y.x);
}
void Emit(InteriorDerivedShell& result,const InteriorCarvedVolume& volume,int axis,int dir,float plane,const Rect& r){
    InteriorShellSurface s;s.sourceModule=volume.moduleIndex;s.axis=axis;s.direction=dir;
    s.corners={Vertex(axis,plane,r.u0,r.v0),Vertex(axis,plane,r.u1,r.v0),
               Vertex(axis,plane,r.u1,r.v1),Vertex(axis,plane,r.u0,r.v1)};
    if(CrossNormal(s)*static_cast<float>(dir)<0)std::swap(s.corners[1],s.corners[3]);
    result.surfaces.push_back(s);
}
bool AxisAligned(float d){return std::isfinite(d)&&std::fabs(std::remainder(d,180.0f))<.0001f;}
bool Supported(const InteriorCarvedVolume& v){
    return AxisAligned(v.yawDegrees)&&AxisAligned(v.pitchDegrees)&&AxisAligned(v.rollDegrees)&&
           std::isfinite(v.center.x)&&std::isfinite(v.center.y)&&std::isfinite(v.center.z)&&
           std::isfinite(v.halfExtents.x)&&std::isfinite(v.halfExtents.y)&&std::isfinite(v.halfExtents.z)&&
           v.halfExtents.x>kEps&&v.halfExtents.y>kEps&&v.halfExtents.z>kEps;
}
struct PortalCut{std::size_t module=0;int axis=0;int direction=0;Rect rect{};};
void EmitLiner(InteriorDerivedShell& out,std::size_t module,int lengthAxis,
               float from,float to,const Rect& opening,int side){
    InteriorShellSurface s;s.sourceModule=module;
    const int u=UAxis(lengthAxis),v=VAxis(lengthAxis);
    const bool onU=side<2;
    s.axis=onU?u:v;s.direction=(side%2)==0?-1:1;
    const float constant=onU?(side==0?opening.u0:opening.u1):
                             (side==2?opening.v0:opening.v1);
    const float lo=onU?opening.v0:opening.u0,hi=onU?opening.v1:opening.u1;
    auto at=[&](float length,float span){Vector3 p{};Put(p,lengthAxis,length);
        Put(p,s.axis,constant);Put(p,onU?v:u,span);return p;};
    s.corners={at(from,lo),at(to,lo),at(to,hi),at(from,hi)};
    if(CrossNormal(s)*static_cast<float>(s.direction)<0)std::swap(s.corners[1],s.corners[3]);
    out.surfaces.push_back(s);
}
}

InteriorDerivedShell ShipInteriorDerivedShellSystem::Build(const ShipInteriorCarvePlan& carve,const WorldScaleProfile& scale){
    InteriorDerivedShell out;
    if(!carve.valid){out.errors.push_back("Cannot build collision shell from an invalid interior carve");return out;}
    if(carve.volumes.empty()){out.errors.push_back("Cannot build shell without walkable volumes");return out;}
    std::unordered_map<std::size_t,const InteriorCarvedVolume*> volumes;
    for(const auto& volume:carve.volumes){
        if(!Supported(volume)){
            out.errors.push_back("Derived shell requires an axis-aligned finite envelope; module "+volume.moduleId+
                " needs a rotation-aware geometry backend (no bogus collision mesh generated)");
        }
        if(!volumes.emplace(volume.moduleIndex,&volume).second)
            out.errors.push_back("Duplicate walkable module index in interior carve");
    }
    if(!out.errors.empty())return out;
    std::vector<PortalCut> cuts;
    for(const auto& portal:carve.portals){
        if(!portal.walkable)continue;
        const auto ia=volumes.find(portal.moduleA),ib=volumes.find(portal.moduleB);
        if(ia==volumes.end()||ib==volumes.end()||ia->second==ib->second)continue;
        const auto& a=*ia->second;const auto& b=*ib->second;
        int axis=0;
        for(int d=1;d<3;++d)if(std::fabs(Component(b.center,d)-Component(a.center,d))>
                                   std::fabs(Component(b.center,axis)-Component(a.center,axis)))axis=d;
        const float delta=Component(b.center,axis)-Component(a.center,axis);
        if(std::fabs(delta)<kEps){
            // Coincident cavity envelopes are resolved by the union below.
            continue;
        }
        const int dir=delta>0?1:-1;
        const float planeA=dir>0?Hi(a,axis):Lo(a,axis);
        const float planeB=dir>0?Lo(b,axis):Hi(b,axis);
        const float gap=(planeB-planeA)*static_cast<float>(dir);
        const int u=UAxis(axis),v=VAxis(axis);
        const float u0=std::max(Lo(a,u),Lo(b,u)),u1=std::min(Hi(a,u),Hi(b,u));
        const float v0=std::max(Lo(a,v),Lo(b,v)),v1=std::min(Hi(a,v),Hi(b,v));
        const float inset=std::max(.12f,scale.referencePlayerHeightMeters*.08f);
        const float width=scale.referenceDoorWidthMeters,height=axis==2?scale.referenceDoorWidthMeters:scale.referenceDoorHeightMeters;
        const float requiredU=width,requiredV=height;
        // A vertical connection requires authored stairs/elevator traversal,
        // not a false horizontal door cut through the deck.
        if(axis==2){out.errors.push_back("Vertical portal requires authored stairs/elevator geometry");continue;}
        // The door threshold MUST coincide with a real walkable floor. A
        // centered-Z door floats above the feet and leaves a phantom wall.
        const float floorA=Lo(a,2),floorB=Lo(b,2);
        if(std::fabs(floorA-floorB)>.03f){
            out.errors.push_back("Portal has mismatched deck floors; add an authored transition");continue;
        }
        const float doorBottom=std::max(floorA,floorB);
        if(doorBottom+requiredV>v1+kEps){
            out.errors.push_back("Portal cannot clear required door height above real deck floor");continue;
        }
        if(u1-u0<requiredU-kEps||v1-v0<requiredV-kEps){
            out.errors.push_back("Portal between "+a.moduleId+" and "+b.moduleId+
                " lacks certified human-scale overlap for an opening");continue;
        }
        if(gap>2.0f*inset+.02f){
            out.errors.push_back("Portal between "+a.moduleId+" and "+b.moduleId+
                " has an unbridged cavity gap; do not cut a fake traversable doorway");continue;
        }
        const float midU=(u0+u1)*.5f,midV=(v0+v1)*.5f;
        if(gap<-kEps){
            InteriorShellOpening opening;opening.moduleA=a.moduleIndex;opening.moduleB=b.moduleIndex;
            opening.axis=axis;opening.center=(a.center+b.center)*.5f;opening.exposedByUnion=true;
            opening.width=requiredU;opening.height=requiredV;
            out.openings.push_back(opening);continue;
        }
        const Rect opening{midU-requiredU*.5f,midU+requiredU*.5f,doorBottom,doorBottom+requiredV};
        cuts.push_back({a.moduleIndex,axis,dir,opening});cuts.push_back({b.moduleIndex,axis,-dir,opening});
        InteriorShellOpening record;record.moduleA=a.moduleIndex;record.moduleB=b.moduleIndex;
        record.axis=axis;record.center=(a.center+b.center)*.5f;
        Put(record.center,u,midU);Put(record.center,v,midV);
        Put(record.center,axis,(planeA+planeB)*.5f);
        record.center.z=doorBottom+requiredV*.5f;
        record.width=requiredU;record.height=requiredV;record.gap=gap;
        out.openings.push_back(record);
        // Bridge the air gap created by the pressure-hull inset. Four shared
        // renderer/collision quads form a continuous passage liner.
        if(gap>kEps)for(int side=0;side<4;++side)
            EmitLiner(out,a.moduleIndex,axis,planeA,planeB,opening,side);
    }
    if(!out.errors.empty()){
        out.openings.clear();return out;
    }
    for(const auto& volume:carve.volumes)for(int axis=0;axis<3;++axis)for(int dir=-1;dir<=1;dir+=2){
        const float plane=dir>0?Hi(volume,axis):Lo(volume,axis);
        std::vector<Rect> pieces{Face(volume,axis)};
        for(const auto& other:carve.volumes){
            if(other.moduleIndex==volume.moduleIndex)continue;
            // If the other cavity continues beyond the plane, this portion
            // is interior to the union: remove the face AND its collider.
            const bool crosses=dir>0?Lo(other,axis)<plane+kEps&&Hi(other,axis)>plane+kEps:
                                        Hi(other,axis)>plane-kEps&&Lo(other,axis)<plane-kEps;
            // Coplanar same-side quads belong to the earliest module only.
            const bool duplicate=other.moduleIndex<volume.moduleIndex&&
                std::fabs((dir>0?Hi(other,axis):Lo(other,axis))-plane)<=kEps;
            if(crosses||duplicate)Subtract(pieces,Face(other,axis));
        }
        for(const auto& cut:cuts)if(cut.module==volume.moduleIndex&&cut.axis==axis&&cut.direction==dir)
            Subtract(pieces,cut.rect);
        for(const auto& r:pieces)Emit(out,volume,axis,dir,plane,r);
    }
    out.ready=true;
    return out;
}
} // namespace subspace
