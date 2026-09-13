#include "world/SpatialFrameSystem.h"
#include <cmath>
#include <unordered_set>
#include <vector>

namespace subspace {
namespace {
Double3 Add(Double3 a,Double3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Double3 Sub(Double3 a,Double3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Double3 Cross(Double3 a,Double3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
DoubleQuat Conjugate(DoubleQuat q){return {-q.x,-q.y,-q.z,q.w};}
DoubleQuat Normalize(DoubleQuat q){
    const double n=std::sqrt(q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w);
    return n<1e-12?DoubleQuat{}:DoubleQuat{q.x/n,q.y/n,q.z/n,q.w/n};
}
DoubleQuat Mul(DoubleQuat a,DoubleQuat b){
    return {a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,
            a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
            a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w,
            a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z};
}
Double3 Rotate(DoubleQuat q,Double3 v){
    q=Normalize(q);
    const DoubleQuat p{v.x,v.y,v.z,0.0};const auto r=Mul(Mul(q,p),Conjugate(q));return {r.x,r.y,r.z};
}
Double3 InverseRotate(DoubleQuat q,Double3 v){return Rotate(Conjugate(Normalize(q)),v);}

struct WorldFrameState {
    Double3 position{};
    DoubleQuat rotation{};
    Double3 linearVelocity{};
    Double3 angularVelocity{};
};

WorldFrameState ResolveWorldFrame(const std::unordered_map<SpatialFrameId,SpatialFrame>& frames,SpatialFrameId id){
    std::vector<const SpatialFrame*> chain;
    std::unordered_set<SpatialFrameId> seen;
    while(id!=InvalidSpatialFrameId&&seen.insert(id).second){
        auto it=frames.find(id);
        if(it==frames.end())break;
        chain.push_back(&it->second);
        id=it->second.parentId;
    }
    WorldFrameState state;
    state.rotation={0.0,0.0,0.0,1.0};
    for(auto it=chain.rbegin();it!=chain.rend();++it){
        const SpatialFrame& f=**it;
        const Double3 offsetWorld=Rotate(state.rotation,f.localPosition);
        state.linearVelocity=Add(state.linearVelocity,
            Add(Cross(state.angularVelocity,offsetWorld),Rotate(state.rotation,f.linearVelocity)));
        state.position=Add(state.position,offsetWorld);
        state.angularVelocity=Add(state.angularVelocity,Rotate(state.rotation,f.angularVelocity));
        state.rotation=Normalize(Mul(state.rotation,Normalize(f.localRotation)));
    }
    return state;
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
    if(id==InvalidSpatialFrameId)return local;
    const auto state=ResolveWorldFrame(frames_,id);
    return Add(state.position,Rotate(state.rotation,local));
}
Double3 SpatialFrameSystem::ToWorldVelocity(SpatialFrameId id,const Double3&localPoint,const Double3&localVelocity) const{
    if(id==InvalidSpatialFrameId)return localVelocity;
    const auto state=ResolveWorldFrame(frames_,id);
    const auto offset=Rotate(state.rotation,localPoint);
    return Add(state.linearVelocity,Add(Cross(state.angularVelocity,offset),Rotate(state.rotation,localVelocity)));
}
SpatialKinematicState SpatialFrameSystem::ReparentPreservingWorld(const SpatialKinematicState&s,SpatialFrameId newParent) const{
    const auto worldPoint=ToWorldPoint(s.frameId,s.localPosition);
    const auto worldVelocity=ToWorldVelocity(s.frameId,s.localPosition,s.localVelocity);
    if(newParent==InvalidSpatialFrameId)return {InvalidSpatialFrameId,worldPoint,worldVelocity};
    const auto parent=ResolveWorldFrame(frames_,newParent);
    const auto worldOffset=Sub(worldPoint,parent.position);
    const auto localPoint=InverseRotate(parent.rotation,worldOffset);
    const auto relativeWorldVelocity=Sub(worldVelocity,
        Add(parent.linearVelocity,Cross(parent.angularVelocity,worldOffset)));
    const auto localVelocity=InverseRotate(parent.rotation,relativeWorldVelocity);
    return {newParent,localPoint,localVelocity};
}
} // namespace subspace
