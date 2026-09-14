#include "rendering/FirstPersonViewSystem.h"
#include <cmath>

namespace subspace {
namespace {
Vector3 Cross(const Vector3&a,const Vector3&b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
Vector3 Safe(Vector3 v,Vector3 fallback){const float l=v.length();return l>1.0e-6f?v*(1.0f/l):fallback;}
void ApplyRoll(Vector3&right,Vector3&up,float roll){const float c=std::cos(roll),s=std::sin(roll);const Vector3 r=right*c+up*s;const Vector3 u=up*c-right*s;right=Safe(r,right);up=Safe(u,up);}
FirstPersonViewPose Build(FirstPersonViewKind kind,const Vector3&pos,float yaw,float pitch,float roll){
    FirstPersonViewPose p;p.kind=kind;p.position=pos;
    const float cp=std::cos(pitch),sp=std::sin(pitch),sy=std::sin(yaw),cy=std::cos(yaw);
    p.forward=Safe({-sy*cp,cy*cp,sp},{0,1,0});
    p.right=Safe({cy,sy,0},{1,0,0});
    p.up=Safe(Cross(p.right,p.forward),{0,0,1});
    ApplyRoll(p.right,p.up,roll);return p;
}
}
FirstPersonViewPose FirstPersonViewSystem::BuildOnFootLocal(const InteriorAvatarState&a){return Build(FirstPersonViewKind::InteriorOnFoot,a.localPosition+Vector3{0,0,a.eyeHeightMeters},a.lookYawRadians,a.lookPitchRadians,0.0f);}
FirstPersonViewPose FirstPersonViewSystem::BuildCockpitLocal(const Vector3&eye,float yaw,float pitch,float roll){auto p=Build(FirstPersonViewKind::Cockpit,eye,yaw,pitch,roll);p.verticalFovDegrees=74.0f;return p;}
} // namespace subspace
