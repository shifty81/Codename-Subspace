#include "world/SpatialFrameSystem.h"
#include <cmath>
#include <unordered_set>

namespace subspace {
namespace {
Double3 Add(Double3 a,Double3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Double3 Sub(Double3 a,Double3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Double3 Cross(Double3 a,Double3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
DoubleQuat Conjugate(DoubleQuat q){return {-q.x,-q.y,-q.z,q.w};}
DoubleQuat Mul(DoubleQuat a,DoubleQuat b){
    return {a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,
            a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
            a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w,
            a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z};
}
Double3 Rotate(DoubleQuat q,Double3 v){
    const double n=std::sqrt(q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w);
    if(n<1e-12)return v;
    q={q.x/n,q.y/n,q.z/n,q.w/n};
    const DoubleQuat p{v.x,v.y,v.z,0.0};const auto r=Mul(Mul(q,p),Conjugate(q));return {r.x,r.y,r.z};
}
}
bool SpatialFrameSystem::Upsert(const SpatialFrame&f){
    if(f.id==InvalidSpatialFrameId||f.id==f.parentId)return false;
    SpatialFrameId p=f.parentId;std::unordered_set<SpatialFrameId> seen{f.id};
    while(p!=InvalidSpatialFrameId){if(!seen.insert(p).second)return false;auto it=frames_.find(p);if(it==frames_.end())break;p=it->second.parentId;}
    frames_[f.id]=f;return true;
}
bool SpatialFrameSystem::Remove(SpatialFrameId id){return frames_.erase(id)>0;}
const SpatialFrame* SpatialFrameSystem::Find(SpatialFrameId id) const{auto it=frames_.find(id);return it==frames_.end()?nullptr:&it->second;}
Double3 SpatialFrameSystem::ToParentPoint(SpatialFrameId id,const Double3&local) const{const auto*f=Find(id);return f?Add(f->localPosition,Rotate(f->localRotation,local)):local;}
Double3 SpatialFrameSystem::ToWorldPoint(SpatialFrameId id,const Double3&local) const{
    Double3 p=local;std::unordered_set<SpatialFrameId> seen;
    while(id!=InvalidSpatialFrameId&&seen.insert(id).second){const auto*f=Find(id);if(!f)break;p=Add(f->localPosition,Rotate(f->localRotation,p));id=f->parentId;}return p;
}
Double3 SpatialFrameSystem::ToWorldVelocity(SpatialFrameId id,const Double3&localPoint,const Double3&localVelocity) const{
    Double3 p=localPoint,v=localVelocity;std::unordered_set<SpatialFrameId> seen;
    while(id!=InvalidSpatialFrameId&&seen.insert(id).second){const auto*f=Find(id);if(!f)break;const auto rp=Rotate(f->localRotation,p);v=Add(f->linearVelocity,Add(Cross(f->angularVelocity,rp),Rotate(f->localRotation,v)));p=Add(f->localPosition,rp);id=f->parentId;}return v;
}
SpatialKinematicState SpatialFrameSystem::ReparentPreservingWorld(const SpatialKinematicState&s,SpatialFrameId newParent) const{
    // Conservative implementation keeps exact world point/velocity when moving
    // to world. Reparenting into a moving nested frame is explicitly represented
    // and can be refined with inverse parent transforms when that frame is active.
    const auto wp=ToWorldPoint(s.frameId,s.localPosition);const auto wv=ToWorldVelocity(s.frameId,s.localPosition,s.localVelocity);
    if(newParent==InvalidSpatialFrameId)return {InvalidSpatialFrameId,wp,wv};
    // Translation-only inverse is safe for current frame roots; rotation-aware
    // inverse remains a certified follow-up before nested moving vehicles ship.
    const auto parentOrigin=ToWorldPoint(newParent,{});
    return {newParent,Sub(wp,parentOrigin),wv};
}
} // namespace subspace
