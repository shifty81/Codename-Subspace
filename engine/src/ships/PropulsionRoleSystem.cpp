#include "ships/PropulsionRoleSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
Vector3 Normalize(Vector3 v,Vector3 fallback){const float l=v.length();return l>1.0e-5f?v*(1.0f/l):fallback;}
const ShipyardAssemblySocket* ExhaustSocket(const ShipyardModuleRecord&r){for(const auto&s:r.sockets)if(s.name=="exhaust")return &s;return nullptr;}
}
PropulsionAxisProfile PropulsionRoleSystem::Infer(const ShipyardModuleRecord&r){
    PropulsionAxisProfile p;p.role=UniversalKitbashAuthority::InferPropulsionRole(r);
    if(const auto* e=ExhaustSocket(r)){p.localExhaustAxis=Normalize({e->dirX,e->dirY,e->dirZ},{0,-1,0});p.confidence=e->manualOverride?1.0f:.82f;p.inferred=!e->manualOverride;}
    else{p.localExhaustAxis={0,-1,0};p.confidence=.30f;}
    p.localThrustAxis=p.localExhaustAxis*-1.0f;
    for(const auto&s:r.sockets)if(s.name=="mount"||s.name=="engine_mount"||s.name=="root"){p.localMountAxis=Normalize({s.dirX,s.dirY,s.dirZ},{0,1,0});break;}
    if(p.role==PropulsionRole::VtolLift||p.role==PropulsionRole::VtolControl||p.role==PropulsionRole::LandingThruster)p.gimbalDegrees=12.0f;
    else if(p.role!=PropulsionRole::MainDrive&&p.role!=PropulsionRole::None)p.gimbalDegrees=18.0f;
    return p;
}
Vector3 PropulsionRoleSystem::TransformDirection(const VisualModulePlacement&p,Vector3 v){
    constexpr float d=3.14159265358979323846f/180.0f;const float yaw=p.yawDegrees*d,pitch=p.pitchDegrees*d,roll=p.rollDegrees*d;
    if(p.mirrorX)v.x=-v.x;if(p.mirrorY)v.y=-v.y;if(p.mirrorZ)v.z=-v.z;
    const float cr=std::cos(roll),sr=std::sin(roll);const float x1=v.x*cr+v.z*sr,y1=v.y,z1=-v.x*sr+v.z*cr;
    const float cp=std::cos(pitch),sp=std::sin(pitch);const float x2=x1,y2=y1*cp-z1*sp,z2=y1*sp+z1*cp;
    const float cy=std::cos(yaw),sy=std::sin(yaw);return Normalize({x2*cy-y2*sy,x2*sy+y2*cy,z2},{0,1,0});
}
PropulsionRole PropulsionRoleSystem::ResolveRoleFromThrust(const Vector3&t0,const Vector3&pos){
    const Vector3 t=Normalize(t0,{0,1,0});const float ax=std::fabs(t.x),ay=std::fabs(t.y),az=std::fabs(t.z);
    if(ay>=ax&&ay>=az)return t.y>=0?PropulsionRole::MainDrive:PropulsionRole::RetroBrake;
    if(az>=ax&&az>=ay)return t.z>=0?PropulsionRole::VtolLift:PropulsionRole::VerticalRcs;
    if(ax>=ay&&ax>=az)return PropulsionRole::LateralRcs;
    const Vector3 radial=Normalize({pos.x,0,pos.z},{1,0,0});if(std::fabs(t.x*radial.x+t.z*radial.z)>.70f)return PropulsionRole::LateralRcs;
    return PropulsionRole::OmnidirectionalRcs;
}
PropulsionPlacementReport PropulsionRoleSystem::Validate(const ShipyardModuleRecord&r,const VisualModulePlacement&p,PropulsionRole requested){
    PropulsionPlacementReport out;const auto profile=Infer(r);out.shipExhaustAxis=TransformDirection(p,profile.localExhaustAxis);out.shipThrustAxis=out.shipExhaustAxis*-1.0f;
    out.resolvedRole=requested==PropulsionRole::None?ResolveRoleFromThrust(out.shipThrustAxis,{p.x,p.y,p.z}):requested;
    const auto dot=[](Vector3 a,Vector3 b){return a.x*b.x+a.y*b.y+a.z*b.z;};
    bool ok=true;std::string why="Propulsion orientation certified";
    switch(out.resolvedRole){
    case PropulsionRole::MainDrive:ok=dot(out.shipThrustAxis,{0,1,0})>=.70f;why=ok?why:"Main-drive thrust must point ship-forward (+Y)";break;
    case PropulsionRole::RetroBrake:ok=dot(out.shipThrustAxis,{0,-1,0})>=.70f;why=ok?why:"Retro thrust must point ship-aft (-Y)";break;
    case PropulsionRole::VtolLift:case PropulsionRole::LandingThruster:case PropulsionRole::VtolControl:ok=dot(out.shipThrustAxis,{0,0,1})>=.65f;why=ok?why:"VTOL / landing thrust must provide dorsal (+Z) lift";break;
    case PropulsionRole::LateralRcs:{const float lateral=std::fabs(out.shipThrustAxis.x);ok=lateral>=.60f;why=ok?why:"Lateral RCS must provide port/starboard translation";}break;
    case PropulsionRole::VerticalRcs:ok=std::fabs(out.shipThrustAxis.z)>=.60f;why=ok?why:"Vertical RCS must provide dorsal/ventral translation";break;
    default:break;
    }
    out.valid=ok;out.message=why;return out;
}
bool PropulsionRoleSystem::ReorientForRole(const ShipyardModuleRecord&r,VisualModulePlacement&p,PropulsionRole requested){
    if(requested==PropulsionRole::None)requested=UniversalKitbashAuthority::InferPropulsionRole(r);
    if(requested==PropulsionRole::None)return false;
    const VisualModulePlacement original=p;
    struct Candidate{float yaw,pitch,roll;};
    static const Candidate candidates[]={
        {0,0,0},{90,0,0},{180,0,0},{270,0,0},
        {0,90,0},{0,-90,0},{90,90,0},{90,-90,0},{180,90,0},{180,-90,0},{270,90,0},{270,-90,0},
        {0,0,90},{0,0,-90},{90,0,90},{90,0,-90},{180,0,90},{180,0,-90},{270,0,90},{270,0,-90},
        {0,180,0},{90,180,0},{180,180,0},{270,180,0}
    };
    float best=-1.0e30f;VisualModulePlacement bestPlacement=original;bool found=false;
    auto scoreFor=[&](const PropulsionPlacementReport& report){
        const auto& t=report.shipThrustAxis;
        switch(requested){
        case PropulsionRole::MainDrive:return t.y;
        case PropulsionRole::RetroBrake:return -t.y;
        case PropulsionRole::VtolLift:case PropulsionRole::VtolControl:case PropulsionRole::LandingThruster:return t.z;
        case PropulsionRole::LateralRcs:return std::fabs(t.x);
        case PropulsionRole::VerticalRcs:return std::fabs(t.z);
        default:return 0.5f*(std::fabs(t.x)+std::fabs(t.z));
        }
    };
    for(const auto& c:candidates){
        VisualModulePlacement q=original;q.yawDegrees=c.yaw;q.pitchDegrees=c.pitch;q.rollDegrees=c.roll;
        const auto report=Validate(r,q,requested);const float score=scoreFor(report);
        if(report.valid&&score>best){best=score;bestPlacement=q;found=true;}
    }
    if(found){p=bestPlacement;return true;}
    return false;
}

} // namespace subspace
