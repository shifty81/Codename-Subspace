#include "fleet/FleetCaptainAiSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kPi = 3.14159265358979323846f;
float Length2D(const Vector3& v){return std::sqrt(v.x*v.x+v.y*v.y);}
Vector3 Normalized2D(const Vector3& v){const float l=Length2D(v);return l>0.0001f?Vector3{v.x/l,v.y/l,0}:Vector3{};}
float Clamp01(float v){return std::clamp(v,0.0f,1.0f);}
float StableUnit(std::uint64_t v){v^=v>>33;v*=0xff51afd7ed558ccdULL;v^=v>>33;v*=0xc4ceb9fe1a85ec53ULL;v^=v>>33;return static_cast<float>(v&0xffffu)/65535.0f;}
Vector3 RotateLocal(const Vector3& local,float yaw){const float s=std::sin(yaw),c=std::cos(yaw);const Vector3 right{c,s,0};const Vector3 forward{-s,c,0};return right*local.x+forward*local.y;}
float DesiredHeading(const Vector3& v,float fallback){return Length2D(v)>.05f?std::atan2(-v.x,v.y):fallback;}
float Shortest(float from,float to){float d=to-from;while(d>kPi)d-=2*kPi;while(d<-kPi)d+=2*kPi;return d;}
}

FleetCaptainProfile FleetCaptainAiSystem::MakeCaptain(std::uint64_t id,FleetShipRole role) const {
    FleetCaptainProfile c;c.captainId=0xCA000000ull+id;
    static const char* first[]={"MARA","ELIAS","TAMSIN","ROOK","LYRA","MORGAN","KAI","SOREN","NOVA","IREN"};
    static const char* last[]={"VALE","MERCER","KESS","ARDEN","ROWAN","DRAKE","SATO","VEGA","QUILL","HALE"};
    c.name=std::string(first[id%10])+" "+last[(id*7+3)%10];
    c.navigation=.58f+StableUnit(id*17+1)*.36f;c.tactics=.48f+StableUnit(id*19+2)*.44f;c.industry=.46f+StableUnit(id*23+3)*.46f;c.discipline=.60f+StableUnit(id*29+4)*.36f;
    switch(role){case FleetShipRole::Mining:c.temperament=FleetCaptainTemperament::Industrial;c.industry=std::max(c.industry,.78f);break;case FleetShipRole::Salvage:c.temperament=FleetCaptainTemperament::Industrial;c.industry=std::max(c.industry,.72f);break;case FleetShipRole::Support:c.temperament=FleetCaptainTemperament::Supportive;c.discipline=std::max(c.discipline,.84f);break;case FleetShipRole::Combat:c.temperament=FleetCaptainTemperament::Aggressive;c.tactics=std::max(c.tactics,.76f);break;default:break;}
    return c;
}

void FleetCaptainAiSystem::EnsureWing(FleetCaptainRuntime& rt,const FleetRuntimeModel& desired,const Vector3& leader,float yaw) const {
    for(const auto& d:desired.ships){auto it=std::find_if(rt.ships.begin(),rt.ships.end(),[&](const FleetAiShipState&s){return s.shipId==d.id;});if(it==rt.ships.end()){FleetAiShipState s;s.shipId=d.id;s.role=d.role;s.captain=MakeCaptain(d.id,d.role);s.mirroredOrder=d.mirroredOrder;s.position=leader+RotateLocal(d.desiredOffset*1.35f,yaw);s.headingRadians=yaw;s.initialized=true;rt.ships.push_back(s);}else{it->role=d.role;it->mirroredOrder=d.mirroredOrder;it->operational=true;}}
    rt.ships.erase(std::remove_if(rt.ships.begin(),rt.ships.end(),[&](const FleetAiShipState&s){return std::none_of(desired.ships.begin(),desired.ships.end(),[&](const FleetRuntimeShip&d){return d.id==s.shipId;});}),rt.ships.end());
}

void FleetCaptainAiSystem::Step(FleetCaptainRuntime& rt,const FleetRuntimeModel& desired,const Vector3& leader,const Vector3& leaderVel,float yaw,const Vector3& selected,bool targetValid,bool vectorActive,bool docked,float dt) const {
    dt=std::clamp(dt,0.001f,.1f);EnsureWing(rt,desired,leader,yaw);
    for(auto& s:rt.ships){const auto dit=std::find_if(desired.ships.begin(),desired.ships.end(),[&](const FleetRuntimeShip&d){return d.id==s.shipId;});if(dit==desired.ships.end())continue;s.mirroredOrder=dit->mirroredOrder;
        const Vector3 slot=leader+RotateLocal(dit->desiredOffset,yaw);s.formationTarget=slot;s.actionTarget=slot;float maxSpeed=19.0f+s.captain.navigation*10.0f;float response=1.6f+s.captain.navigation*2.6f;
        if(docked){s.state=FleetCaptainState::Holding;s.behavior="HOLDING FOR LEADER";s.actionTarget=leader;s.velocity=s.velocity*(1.0f-std::min(1.0f,dt*3.5f));}
        else if(vectorActive||s.mirroredOrder==StrategicOrderKind::VectorTo){s.state=FleetCaptainState::VectorSync;s.behavior="VECTOR SYNC";s.actionTarget=leader+RotateLocal({0,-10.0f,0},yaw);maxSpeed=42.0f;response=5.0f;}
        else if(s.mirroredOrder==StrategicOrderKind::Engage&&targetValid){s.state=s.role==FleetShipRole::Support?FleetCaptainState::Supporting:FleetCaptainState::Engaging;s.behavior=s.role==FleetShipRole::Support?"SUPPORTING COMBAT":"ENGAGING LEADER TARGET";const float standoff=s.role==FleetShipRole::Support?18.0f:10.0f;s.actionTarget=selected+RotateLocal({(s.shipId%2?1.0f:-1.0f)*standoff,-standoff,0},yaw);maxSpeed+=s.captain.tactics*6.0f;}
        else if(s.mirroredOrder==StrategicOrderKind::Mine&&targetValid&&s.role==FleetShipRole::Mining){s.state=FleetCaptainState::Mining;s.behavior="MINING LEADER TARGET";s.actionTarget=selected+RotateLocal({6.0f,-8.0f,0},yaw);maxSpeed=16.0f;}
        else if(s.mirroredOrder==StrategicOrderKind::Salvage&&targetValid&&s.role==FleetShipRole::Salvage){s.state=FleetCaptainState::Salvaging;s.behavior="SALVAGING LEADER TARGET";s.actionTarget=selected+RotateLocal({-6.0f,-7.0f,0},yaw);maxSpeed=16.0f;}
        else if(s.mirroredOrder==StrategicOrderKind::Dock){s.state=FleetCaptainState::Docking;s.behavior="DOCK QUEUE";s.actionTarget=slot;maxSpeed=12.0f;}
        else {const float gap=Length2D(slot-s.position);s.state=gap>desired.spacing*2.4f?FleetCaptainState::CatchingUp:FleetCaptainState::Following;s.behavior=s.state==FleetCaptainState::CatchingUp?"CATCHING FORMATION":"FOLLOWING LEADER";if(gap>desired.spacing*2.4f)maxSpeed*=1.55f;}
        const Vector3 to=s.actionTarget-s.position;const float dist=Length2D(to);if(!docked){const Vector3 dir=Normalized2D(to);const float arrival=Clamp01(dist/std::max(4.0f,desired.spacing*.85f));const Vector3 targetVel=dir*(maxSpeed*arrival)+leaderVel*(s.state==FleetCaptainState::Following?.82f:.25f);const float alpha=1.0f-std::exp(-dt*response);s.velocity=s.velocity+(targetVel-s.velocity)*alpha;s.position=s.position+s.velocity*dt;const float want=DesiredHeading(s.velocity,s.headingRadians);s.headingRadians+=Shortest(s.headingRadians,want)*(1.0f-std::exp(-dt*(2.0f+s.captain.discipline*3.0f)));}
    }
}
const char* FleetCaptainAiSystem::StateName(FleetCaptainState s){switch(s){case FleetCaptainState::Forming:return"FORMING";case FleetCaptainState::Following:return"FOLLOWING";case FleetCaptainState::CatchingUp:return"CATCHING UP";case FleetCaptainState::Engaging:return"ENGAGING";case FleetCaptainState::Mining:return"MINING";case FleetCaptainState::Salvaging:return"SALVAGING";case FleetCaptainState::Supporting:return"SUPPORTING";case FleetCaptainState::Docking:return"DOCKING";case FleetCaptainState::VectorSync:return"VECTOR SYNC";case FleetCaptainState::Holding:return"HOLDING";}return"UNKNOWN";}
} // namespace subspace
