#include "content/ShipyardGeometryAnalysisSystem.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace subspace {
namespace {

Vector3 Remap(const Vector3& v) { return {v.x,v.z,v.y}; }
Vector3 Cross(const Vector3& a,const Vector3& b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
float Dot(const Vector3& a,const Vector3& b){return a.x*b.x+a.y*b.y+a.z*b.z;}

struct TriSample {
    Vector3 centroid{};
    Vector3 normal{};
    float area=0.0f;
};

float Axis(const Vector3& v,int axis){return axis==0?v.x:(axis==1?v.y:v.z);}

ShipyardSurfaceContact ContactFor(const std::vector<TriSample>& samples,
                                  const Vector3& minv,const Vector3& maxv,
                                  int axis,float sign){
    ShipyardSurfaceContact out;
    const float lo=Axis(minv,axis), hi=Axis(maxv,axis), span=std::max(1.0e-5f,hi-lo);
    constexpr int kBins=18;
    std::array<float,kBins> score{};
    std::array<float,kBins> area{};
    const Vector3 target=axis==0?Vector3{sign,0,0}:(axis==1?Vector3{0,sign,0}:Vector3{0,0,sign});
    float candidateArea=0.0f;

    for(const auto&t:samples){
        const float coord=Axis(t.centroid,axis);
        const float normalized=std::clamp((coord-lo)/span,0.0f,1.0f);
        const float proximity=sign>0?normalized:1.0f-normalized;
        const float facing=std::fabs(Dot(t.normal,target));
        if(facing<0.20f) continue;
        int bin=std::clamp(static_cast<int>(normalized*static_cast<float>(kBins)),0,kBins-1);
        // Area is authoritative; direction proximity is only a bias. This is
        // what prevents a tiny far-away protrusion from redefining the socket.
        const float w=t.area*(0.55f+0.45f*facing)*(0.72f+0.78f*proximity);
        score[bin]+=w; area[bin]+=t.area; candidateArea+=t.area;
    }
    if(candidateArea<=1.0e-7f) return out;

    int best=-1; float bestScore=-1.0f;
    for(int i=0;i<kBins;++i){
        // Include adjacent bins so authored bevels around a mating plane count
        // together while tiny isolated extrema do not dominate.
        float s=score[i]; if(i>0)s+=score[i-1]*0.45f; if(i+1<kBins)s+=score[i+1]*0.45f;
        if(s>bestScore){bestScore=s;best=i;}
    }
    if(best<0||bestScore<=0.0f) return out;

    Vector3 p{}; float sum=0.0f; float supporting=0.0f;
    for(const auto&t:samples){
        const float normalized=std::clamp((Axis(t.centroid,axis)-lo)/span,0.0f,1.0f);
        const int bin=std::clamp(static_cast<int>(normalized*static_cast<float>(kBins)),0,kBins-1);
        if(std::abs(bin-best)>1) continue;
        const float facing=std::fabs(Dot(t.normal,target)); if(facing<0.20f)continue;
        const float w=t.area*(0.5f+0.5f*facing);
        p=p+t.centroid*w; sum+=w; supporting+=t.area;
    }
    if(sum<=1.0e-7f) return out;
    p=p*(1.0f/sum);
    out.point=p;
    out.normal=target;
    out.supportingArea=supporting;
    const float concentration=std::clamp(supporting/std::max(candidateArea,1.0e-6f),0.0f,1.0f);
    out.confidence=std::clamp(0.45f+0.55f*concentration,0.0f,1.0f);
    out.valid=true;
    return out;
}

} // namespace

ShipyardSurfaceContactSet ShipyardGeometryAnalysisSystem::Analyze(const ObjMeshData& mesh){
    ShipyardSurfaceContactSet out;
    if(mesh.positions.empty()||mesh.triangles.empty())return out;
    Vector3 minv=Remap(mesh.positions.front()),maxv=minv;
    for(const auto&src:mesh.positions){
        const auto v=Remap(src);
        minv.x=std::min(minv.x,v.x);minv.y=std::min(minv.y,v.y);minv.z=std::min(minv.z,v.z);
        maxv.x=std::max(maxv.x,v.x);maxv.y=std::max(maxv.y,v.y);maxv.z=std::max(maxv.z,v.z);
    }
    std::vector<TriSample> samples;samples.reserve(mesh.triangles.size());
    for(const auto&tri:mesh.triangles){
        const auto a=Remap(mesh.positions[tri.position[0]]),b=Remap(mesh.positions[tri.position[1]]),c=Remap(mesh.positions[tri.position[2]]);
        const auto cr=Cross(b-a,c-a);const float len=cr.length(); if(len<=1.0e-7f)continue;
        samples.push_back({(a+b+c)*(1.0f/3.0f),cr*(1.0f/len),len*0.5f});
    }
    out.forward=ContactFor(samples,minv,maxv,1,1.0f);
    out.aft=ContactFor(samples,minv,maxv,1,-1.0f);
    out.port=ContactFor(samples,minv,maxv,0,-1.0f);
    out.starboard=ContactFor(samples,minv,maxv,0,1.0f);
    out.dorsal=ContactFor(samples,minv,maxv,2,1.0f);
    out.ventral=ContactFor(samples,minv,maxv,2,-1.0f);
    return out;
}

} // namespace subspace
