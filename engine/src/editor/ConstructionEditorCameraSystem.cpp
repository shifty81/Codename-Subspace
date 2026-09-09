#include "editor/ConstructionEditorCameraSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kPi=3.14159265358979323846f;
float Rad(float d){return d*kPi/180.0f;}
Vector3 Normalize(Vector3 v,Vector3 fallback){const float l=v.length();return l>1e-5f?v*(1.0f/l):fallback;}
Vector3 Cross(const Vector3&a,const Vector3&b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
void RebuildOrbitEye(ConstructionEditorCameraState&s){
    const float yaw=Rad(s.yawDegrees),pitch=Rad(s.pitchDegrees);const float cp=std::cos(pitch),sp=std::sin(pitch);
    const Vector3 fromTarget{std::sin(yaw)*cp,-std::cos(yaw)*cp,sp};
    s.eye=s.assemblyCenter+fromTarget*std::max(.25f,s.orbitDistance);s.forward=Normalize(s.assemblyCenter-s.eye,{0,1,-.2f});
}
void FreeBasis(const ConstructionEditorCameraState&s,Vector3&right,Vector3&up){
    Vector3 worldUp{0,0,1};if(std::fabs(s.forward.z)>.985f)worldUp={0,1,0};right=Normalize(Cross(s.forward,worldUp),{1,0,0});up=Normalize(Cross(right,s.forward),{0,0,1});
    if(std::fabs(s.rollDegrees)>.001f){const float r=Rad(s.rollDegrees),c=std::cos(r),sn=std::sin(r);const auto rr=right*c+up*sn;const auto uu=up*c-right*sn;right=Normalize(rr,right);up=Normalize(uu,up);}
}
}
void ConstructionEditorCameraSystem::Reset(ConstructionEditorCameraState&s,const Vector3&center,float radius){s={};s.assemblyCenter=center;s.orbitDistance=std::clamp(radius*2.8f,4.0f,160.0f);s.moveSpeed=std::clamp(radius*1.5f,2.0f,120.0f);RebuildOrbitEye(s);}
void ConstructionEditorCameraSystem::SetAssemblyCenter(ConstructionEditorCameraState&s,const Vector3&center,bool preserveEye){const Vector3 delta=center-s.assemblyCenter;s.assemblyCenter=center;if(!preserveEye)s.eye=s.eye+delta;if(s.mode==ConstructionCameraMode::CenteredInspect&&s.centerLock)s.forward=Normalize(center-s.eye,s.forward);}
void ConstructionEditorCameraSystem::Orbit(ConstructionEditorCameraState&s,float dyaw,float dpitch){s.mode=ConstructionCameraMode::CenteredInspect;s.yawDegrees+=dyaw;s.pitchDegrees=std::clamp(s.pitchDegrees+dpitch,-89.0f,89.0f);RebuildOrbitEye(s);}
void ConstructionEditorCameraSystem::TruckPedestal(ConstructionEditorCameraState&s,float dr,float du){s.mode=ConstructionCameraMode::CenteredInspect;Vector3 right,up;FreeBasis(s,right,up);s.eye=s.eye+right*dr+up*du;s.orbitDistance=std::max(.25f,(s.eye-s.assemblyCenter).length());s.forward=Normalize(s.assemblyCenter-s.eye,s.forward);}
void ConstructionEditorCameraSystem::BeginFreeFly(ConstructionEditorCameraState&s){if(s.mode==ConstructionCameraMode::FreeFly)return;s.mode=ConstructionCameraMode::FreeFly;s.forward=Normalize(s.assemblyCenter-s.eye,s.forward);}
void ConstructionEditorCameraSystem::EndFreeFly(ConstructionEditorCameraState&s){if(s.mode!=ConstructionCameraMode::FreeFly)return;s.mode=ConstructionCameraMode::CenteredInspect;s.orbitDistance=std::max(.25f,(s.eye-s.assemblyCenter).length());s.forward=Normalize(s.assemblyCenter-s.eye,s.forward);const Vector3 d=(s.eye-s.assemblyCenter)*(1.0f/s.orbitDistance);s.pitchDegrees=std::asin(std::clamp(d.z,-1.0f,1.0f))*180.0f/kPi;s.yawDegrees=std::atan2(d.x,-d.y)*180.0f/kPi;}
void ConstructionEditorCameraSystem::Look(ConstructionEditorCameraState&s,float dyaw,float dpitch){BeginFreeFly(s);const float yaw=std::atan2(s.forward.x,s.forward.y)+Rad(dyaw);const float pitch=std::asin(std::clamp(s.forward.z,-1.0f,1.0f))+Rad(dpitch);const float p=std::clamp(pitch,Rad(-89.0f),Rad(89.0f));const float cp=std::cos(p);s.forward=Normalize({std::sin(yaw)*cp,std::cos(yaw)*cp,std::sin(p)},s.forward);}
void ConstructionEditorCameraSystem::Roll(ConstructionEditorCameraState&s,float d){BeginFreeFly(s);s.rollDegrees+=d;while(s.rollDegrees>180)s.rollDegrees-=360;while(s.rollDegrees<-180)s.rollDegrees+=360;}
void ConstructionEditorCameraSystem::MoveFree(ConstructionEditorCameraState&s,float f,float r,float u,float dt){BeginFreeFly(s);Vector3 right,up;FreeBasis(s,right,up);const float speed=s.moveSpeed*std::max(0.0f,dt);s.eye=s.eye+s.forward*(f*speed)+right*(r*speed)+up*(u*speed);}
void ConstructionEditorCameraSystem::AdjustSpeed(ConstructionEditorCameraState&s,float steps){s.moveSpeed=std::clamp(s.moveSpeed*std::exp(steps*.18f),.25f,500.0f);}
void ConstructionEditorCameraSystem::Dolly(ConstructionEditorCameraState&s,float steps){
    if(s.mode==ConstructionCameraMode::FreeFly){s.eye=s.eye+s.forward*(steps*std::max(.25f,s.moveSpeed)*.35f);return;}
    s.orbitDistance=std::clamp(s.orbitDistance*std::exp(-steps*.18f),.35f,5000.0f);
    RebuildOrbitEye(s);
}
Vector3 ConstructionEditorCameraSystem::Target(const ConstructionEditorCameraState&s){return s.mode==ConstructionCameraMode::FreeFly?s.eye+s.forward*std::max(2.0f,s.orbitDistance):s.assemblyCenter;}
} // namespace subspace
