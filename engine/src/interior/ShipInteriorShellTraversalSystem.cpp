#include "interior/ShipInteriorShellTraversalSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kEps=0.0001f;
float Axis(const Vector3& p,int a){return a==0?p.x:(a==1?p.y:p.z);}
float Low(const InteriorCarvedVolume& v,int a){return Axis(v.center,a)-Axis(v.halfExtents,a);}
float High(const InteriorCarvedVolume& v,int a){return Axis(v.center,a)+Axis(v.halfExtents,a);}
const InteriorCarvedVolume* Find(const ShipInteriorCarvePlan& carve,std::size_t index){
    for(const auto& v:carve.volumes)if(v.moduleIndex==index)return &v;
    return nullptr;
}
bool InVolume(const InteriorCarvedVolume& v,const Vector3& p,float height){
    // No arbitrary multi-deck teleportation. Floor and head clearance must be
    // present in one authored cavity; XY wall distance is checked separately
    // against the actual SUBTRACTED shell quads (not source hull AABBs).
    return p.x>=Low(v,0)-kEps&&p.x<=High(v,0)+kEps&&
           p.y>=Low(v,1)-kEps&&p.y<=High(v,1)+kEps&&
           p.z>=Low(v,2)-.03f&&p.z<=Low(v,2)+.03f&&
           p.z+height<=High(v,2)+kEps;
}
bool InBridge(const ShipInteriorCarvePlan& carve,const InteriorShellOpening& o,
              const Vector3& p,float radius,float height){
    if(o.exposedByUnion||o.gap<=kEps||o.axis==2)return false;
    const auto* a=Find(carve,o.moduleA);const auto* b=Find(carve,o.moduleB);
    if(!a||!b)return false;
    const int axis=o.axis, lateral=axis==0?1:0;
    // Build() records direction from A to B; for reversed endpoints derive
    // the facing planes rather than assuming module-index spatial order.
    const float faceA=Axis(b->center,axis)>Axis(a->center,axis)?High(*a,axis):Low(*a,axis);
    const float faceB=Axis(b->center,axis)>Axis(a->center,axis)?Low(*b,axis):High(*b,axis);
    if(Axis(p,axis)<std::min(faceA,faceB)-kEps||Axis(p,axis)>std::max(faceA,faceB)+kEps)return false;
    if(std::fabs(Axis(p,lateral)-Axis(o.center,lateral))>o.width*.5f-radius+kEps)return false;
    const float floor=std::max(Low(*a,2),Low(*b,2));
    return std::fabs(p.z-floor)<=.03f&&p.z+height<=std::min(High(*a,2),High(*b,2))+kEps;
}
float DistanceSquaredToWall(const InteriorShellSurface& s,const Vector3& p){
    const float x0=std::min(s.corners[0].x,s.corners[2].x),x1=std::max(s.corners[0].x,s.corners[2].x);
    const float y0=std::min(s.corners[0].y,s.corners[2].y),y1=std::max(s.corners[0].y,s.corners[2].y);
    const float dx=p.x-std::clamp(p.x,x0,x1),dy=p.y-std::clamp(p.y,y0,y1);
    return dx*dx+dy*dy;
}
}
bool ShipInteriorShellTraversalSystem::CanOccupy(const ShipInteriorCarvePlan& carve,
        const InteriorDerivedShell& shell,Vector3 p,float radius,float height){
    if(!carve.valid||!shell.ready||shell.surfaces.empty()||
       !std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)||
       !std::isfinite(radius)||!std::isfinite(height)||radius<=0||height<=0)return false;
    bool inside=false;
    for(const auto& v:carve.volumes)if(InVolume(v,p,height)){inside=true;break;}
    if(!inside)for(const auto& o:shell.openings)if(InBridge(carve,o,p,radius,height)){inside=true;break;}
    if(!inside)return false;
    for(const auto& surface:shell.surfaces){
        if(!surface.blocksMovement)continue;
        float minZ=surface.corners[0].z,maxZ=minZ;
        for(const auto& q:surface.corners){minZ=std::min(minZ,q.z);maxZ=std::max(maxZ,q.z);}
        if(surface.axis==2||p.z>=maxZ-kEps||p.z+height<=minZ+kEps)continue;
        if(DistanceSquaredToWall(surface,p)<radius*radius-kEps)return false;
    }
    return true;
}
Vector3 ShipInteriorShellTraversalSystem::Move(const ShipInteriorCarvePlan& carve,
        const InteriorDerivedShell& shell,Vector3 feet,Vector3 delta,float radius,float height){
    if(!CanOccupy(carve,shell,feet,radius,height))return feet;
    if(!std::isfinite(delta.x)||!std::isfinite(delta.y)||!std::isfinite(delta.z))return feet;
    // Deck changes require authored stairs/elevators; never allow free Z motion.
    delta.z=0;
    const float distance=std::hypot(delta.x,delta.y);
    if(distance<=kEps)return feet;
    // This API is frame locomotion, not teleportation. Reject very large
    // deltas rather than allowing a clamped step count to tunnel a thin wall.
    if(distance>10.0f)return feet;
    const int steps=std::clamp(static_cast<int>(std::ceil(distance/.04f)),1,256);
    const Vector3 step=delta*(1.0f/static_cast<float>(steps));
    for(int i=0;i<steps;++i){
        const auto full=feet+step;
        if(CanOccupy(carve,shell,full,radius,height)){feet=full;continue;}
        const Vector3 x=feet+Vector3{step.x,0,0};
        if(CanOccupy(carve,shell,x,radius,height))feet=x;
        const Vector3 y=feet+Vector3{0,step.y,0};
        if(CanOccupy(carve,shell,y,radius,height))feet=y;
    }
    return feet;
}
bool ShipInteriorShellTraversalSystem::Spawn(const ShipInteriorCarvePlan& carve,
        const InteriorDerivedShell& shell,float radius,float height,Vector3& feet){
    if(CanOccupy(carve,shell,feet,radius,height))return true;
    for(const auto& v:carve.volumes){
        const Vector3 candidate{v.center.x,v.center.y,Low(v,2)};
        if(CanOccupy(carve,shell,candidate,radius,height)){feet=candidate;return true;}
    }
    return false;
}
} // namespace subspace
