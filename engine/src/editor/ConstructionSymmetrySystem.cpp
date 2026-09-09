#include "editor/ConstructionSymmetrySystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
float Normalize180(float d){while(d>180.0f)d-=360.0f;while(d<-180.0f)d+=360.0f;return d;}
std::string SwapOnce(const std::string& name,const char* a,const char* b){
    const auto p=name.find(a); if(p==std::string::npos)return {};
    std::string out=name;out.replace(p,std::char_traits<char>::length(a),b);return out;
}
}

const char* ConstructionSymmetrySystem::AxisName(ConstructionSymmetryAxis axis){
    switch(axis){
    case ConstructionSymmetryAxis::PortStarboard:return "PORT <-> STARBOARD";
    case ConstructionSymmetryAxis::ForeAft:return "FORE <-> AFT";
    case ConstructionSymmetryAxis::DorsalVentral:return "DORSAL <-> VENTRAL";
    }
    return "SYMMETRY";
}

Vector3 ConstructionSymmetrySystem::ReflectPoint(const Vector3&p,const ConstructionSymmetryFrame&f){
    Vector3 out=p;
    if(f.axis==ConstructionSymmetryAxis::PortStarboard)out.x=2.0f*f.origin.x-p.x;
    else if(f.axis==ConstructionSymmetryAxis::ForeAft)out.y=2.0f*f.origin.y-p.y;
    else out.z=2.0f*f.origin.z-p.z;
    return out;
}

Vector3 ConstructionSymmetrySystem::ReflectDirection(const Vector3&v,ConstructionSymmetryAxis a){
    Vector3 out=v;
    if(a==ConstructionSymmetryAxis::PortStarboard)out.x=-out.x;
    else if(a==ConstructionSymmetryAxis::ForeAft)out.y=-out.y;
    else out.z=-out.z;
    return out;
}

VisualModulePlacement ConstructionSymmetrySystem::ReflectPlacement(const VisualModulePlacement&p,const ConstructionSymmetryFrame&f){
    VisualModulePlacement out=p;
    const auto reflected=ReflectPoint({p.x,p.y,p.z},f);out.x=reflected.x;out.y=reflected.y;out.z=reflected.z;
    // M*R*M converts the source orientation into the opposite-handed partner,
    // while the corresponding local mirror flag performs the actual mesh
    // handedness reflection. This is deliberately not a 180-degree rotate.
    if(f.axis==ConstructionSymmetryAxis::PortStarboard){
        out.pitchDegrees=Normalize180(p.pitchDegrees);
        out.yawDegrees=Normalize180(-p.yawDegrees);
        out.rollDegrees=Normalize180(-p.rollDegrees);
        out.mirrorX=!p.mirrorX;
    }else if(f.axis==ConstructionSymmetryAxis::ForeAft){
        out.pitchDegrees=Normalize180(-p.pitchDegrees);
        out.yawDegrees=Normalize180(-p.yawDegrees);
        out.rollDegrees=Normalize180(p.rollDegrees);
        out.mirrorY=!p.mirrorY;
    }else{
        out.pitchDegrees=Normalize180(-p.pitchDegrees);
        out.yawDegrees=Normalize180(p.yawDegrees);
        out.rollDegrees=Normalize180(-p.rollDegrees);
        out.mirrorZ=!p.mirrorZ;
    }
    return out;
}

ShipyardAssemblySocket ConstructionSymmetrySystem::ReflectSocket(const ShipyardAssemblySocket&s,ConstructionSymmetryAxis a){
    ShipyardAssemblySocket out=s;
    const auto pos=ReflectDirection({s.x,s.y,s.z},a);out.x=pos.x;out.y=pos.y;out.z=pos.z;
    const auto dir=ReflectDirection({s.dirX,s.dirY,s.dirZ},a);out.dirX=dir.x;out.dirY=dir.y;out.dirZ=dir.z;
    const auto up=ReflectDirection({s.upX,s.upY,s.upZ},a);out.upX=up.x;out.upY=up.y;out.upZ=up.z;
    out.name=ReflectSocketName(s.name,a);out.manualOverride=true;return out;
}

std::string ConstructionSymmetrySystem::ReflectSocketName(const std::string&n,ConstructionSymmetryAxis a){
    if(a==ConstructionSymmetryAxis::PortStarboard){
        if(auto v=SwapOnce(n,"starboard","port");!v.empty())return v;
        if(auto v=SwapOnce(n,"port","starboard");!v.empty())return v;
        if(auto v=SwapOnce(n,"right","left");!v.empty())return v;
        if(auto v=SwapOnce(n,"left","right");!v.empty())return v;
    }else if(a==ConstructionSymmetryAxis::ForeAft){
        if(auto v=SwapOnce(n,"forward","aft");!v.empty())return v;
        if(auto v=SwapOnce(n,"aft","forward");!v.empty())return v;
        if(auto v=SwapOnce(n,"front","rear");!v.empty())return v;
        if(auto v=SwapOnce(n,"rear","front");!v.empty())return v;
        if(auto v=SwapOnce(n,"bow","stern");!v.empty())return v;
        if(auto v=SwapOnce(n,"stern","bow");!v.empty())return v;
    }else{
        if(auto v=SwapOnce(n,"dorsal","ventral");!v.empty())return v;
        if(auto v=SwapOnce(n,"ventral","dorsal");!v.empty())return v;
        if(auto v=SwapOnce(n,"top","bottom");!v.empty())return v;
        if(auto v=SwapOnce(n,"bottom","top");!v.empty())return v;
        if(auto v=SwapOnce(n,"up","down");!v.empty())return v;
        if(auto v=SwapOnce(n,"down","up");!v.empty())return v;
    }
    return n;
}

ConstructionSymmetryValidation ConstructionSymmetrySystem::ValidatePair(const VisualModulePlacement&a,const VisualModulePlacement&b,const ConstructionSymmetryFrame&f,float tol){
    ConstructionSymmetryValidation out;const auto expected=ReflectPlacement(a,f);
    const float dx=expected.x-b.x,dy=expected.y-b.y,dz=expected.z-b.z;out.positionError=std::sqrt(dx*dx+dy*dy+dz*dz);
    const bool handed=(expected.mirrorX==b.mirrorX&&expected.mirrorY==b.mirrorY&&expected.mirrorZ==b.mirrorZ);
    out.valid=out.positionError<=std::max(.0001f,tol)&&handed;
    out.message=out.valid?"Exact reflected symmetry pair":"Symmetry partner position/handedness mismatch";return out;
}

} // namespace subspace
