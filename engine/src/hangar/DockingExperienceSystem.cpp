#include "hangar/DockingExperienceSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
float Dist(const Vector3&a,const Vector3&b){return (a-b).length();}
Vector3 Lerp(const Vector3&a,const Vector3&b,float t){return a+(b-a)*t;}
float Clamp01(float v){return std::clamp(v,0.0f,1.0f);}
void UpdateGuidance(DockingExperienceState& s,const Vector3& ship,const Vector3& target,float radius){
    const Vector3 toTarget=target-ship;
    s.distanceToGuidance=toTarget.length();
    s.insideCorridor=s.distanceToGuidance<=std::max(0.1f,radius);
    const Vector3 corridorForward=(s.geometry.apertureWorld-s.geometry.captureWorld).normalized();
    const Vector3 approachDir=s.distanceToGuidance>0.001f?toTarget.normalized():corridorForward;
    s.alignment=Clamp01(((approachDir.x*corridorForward.x+approachDir.y*corridorForward.y+approachDir.z*corridorForward.z)+1.0f)*0.5f);
    const Vector3 stationToShip=ship-s.stationWorld;
    const Vector3 stationToTarget=target-s.stationWorld;
    s.lateralError=(stationToShip-stationToTarget).length();
    if(!s.trafficClearance)s.guidanceCue="HOLD / TRAFFIC";
    else if(s.alignment<0.58f)s.guidanceCue="ALIGN WITH CORRIDOR";
    else if(!s.insideCorridor)s.guidanceCue="ENTER GUIDANCE GATE";
    else if(s.assignedSpeedLimit<=8.0f)s.guidanceCue="FINAL / MINIMUM SPEED";
    else s.guidanceCue="CORRIDOR GREEN";
}
}

bool DockingExperienceSystem::Request(DockingExperienceState& s,std::uint64_t stationId,const Vector3& station,const Vector3& ship,bool autoDock) const {
    if(stationId==0||s.stage!=DockingExperienceStage::Undocked)return false;
    s={};s.stationId=stationId;s.stationWorld=station;s.autoDock=autoDock;
    s.geometry=StationDockingGeometrySystem{}.Build(stationId,station,ship,StationBerthSize::Standard);
    s.berthId=s.geometry.berthId;
    // Join the physical approach corridor at the nearest sensible gate. A ship
    // already inside the outer docking envelope must never reverse away from
    // the station just to visit the first/outermost waypoint.
    s.corridorWaypoint=0;
    if(!s.geometry.corridor.empty()){
        float best=Dist(ship,s.geometry.corridor.front().position);
        for(std::size_t i=1;i<s.geometry.corridor.size();++i){
            const float candidate=Dist(ship,s.geometry.corridor[i].position);
            if(candidate<best){best=candidate;s.corridorWaypoint=i;}
        }
    }
    const auto* initial=s.geometry.corridor.empty()?nullptr:&s.geometry.corridor[s.corridorWaypoint];
    s.approachWorld=initial?initial->position:s.geometry.captureWorld;
    s.corridorRadius=initial?initial->radius:4.0;
    s.assignedSpeedLimit=initial?initial->speedLimit:18.0f;
    s.assignedForward=(s.geometry.apertureWorld-s.geometry.captureWorld).normalized();
    s.trafficClearance=true;s.stage=DockingExperienceStage::Requested;
    UpdateGuidance(s,ship,s.approachWorld,static_cast<float>(s.corridorRadius));
    s.status="DOCKING CLEARED "+s.berthId+" / FOLLOW GUIDANCE LIGHTS";return true;
}

Vector3 DockingExperienceSystem::Update(DockingExperienceState& s,const Vector3& ship,double dt) const {
    if(dt<=0)return ship;Vector3 result=ship;
    if(s.stage==DockingExperienceStage::Requested){s.stage=DockingExperienceStage::Approach;s.status=s.autoDock?"AUTO DOCK / APPROACH CORRIDOR":"MANUAL DOCK / ENTER APPROACH CORRIDOR";}
    if(s.stage==DockingExperienceStage::Approach){
        if(s.geometry.corridor.empty()){s.stage=DockingExperienceStage::Capture;return result;}
        s.corridorWaypoint=std::min(s.corridorWaypoint,s.geometry.corridor.size()-1);
        const auto& waypoint=s.geometry.corridor[s.corridorWaypoint];s.approachWorld=waypoint.position;s.corridorRadius=waypoint.radius;s.assignedSpeedLimit=waypoint.speedLimit;
        const float d=Dist(ship,waypoint.position);UpdateGuidance(s,ship,waypoint.position,waypoint.radius);const double base=double(s.corridorWaypoint)/double(s.geometry.corridor.size());s.progress=std::clamp(base*.78+(1.0-std::min(1.0f,d/45.0f))*.18,0.0,.78);
        if(s.autoDock)result=Lerp(ship,waypoint.position,static_cast<float>(std::min(1.0,dt*(s.corridorWaypoint+1==s.geometry.corridor.size()?.58:.82))));
        if(d<=waypoint.radius){
            if(s.corridorWaypoint+1<s.geometry.corridor.size()){++s.corridorWaypoint;s.status="DOCKING "+s.berthId+" / NEXT GUIDANCE GATE";}
            else {s.stage=DockingExperienceStage::Capture;s.status="DOCKING CAPTURE / REDUCE SPEED";s.progress=.80;}
        }
    }
    else if(s.stage==DockingExperienceStage::Capture){
        s.assignedSpeedLimit=4.0f;UpdateGuidance(s,ship,s.geometry.apertureWorld,0.75f);s.guidanceCue="CAPTURE / HOLD CENTERLINE";
        s.progress=std::min(1.0,s.progress+dt*.34);s.captureStrength=std::clamp((s.progress-.80)/.20,0.0,1.0);
        // Capture terminates at the physical aperture, never the station center.
        result=Lerp(ship,s.geometry.apertureWorld,static_cast<float>(std::min(1.0,dt*.64)));
        if(s.progress>=.995||Dist(result,s.geometry.apertureWorld)<.28f){s.stage=DockingExperienceStage::Docked;s.status="DOCKED "+s.berthId;s.progress=1.0;s.captureStrength=1.0;s.hangarReady=true;s.distanceToGuidance=0.0f;s.insideCorridor=true;s.guidanceCue="BERTH SECURED";result=s.geometry.apertureWorld;}
    }
    else if(s.stage==DockingExperienceStage::Undocking){
        s.assignedSpeedLimit=10.0f;UpdateGuidance(s,ship,s.geometry.undockWorld,4.0f);s.guidanceCue="EXIT CORRIDOR";
        s.progress=std::max(0.0,s.progress-dt*.42);result=Lerp(ship,s.geometry.undockWorld,static_cast<float>(std::min(1.0,dt*.72)));
        if(s.progress<=.01||Dist(result,s.geometry.undockWorld)<.35f){s.stage=DockingExperienceStage::Undocked;s.status="UNDOCKED";s.stationId=0;s.trafficClearance=false;s.hangarReady=false;s.captureStrength=0.0;result=s.geometry.undockWorld;}
    }
    return result;
}

bool DockingExperienceSystem::RequestUndock(DockingExperienceState& s) const {
    if(s.stage!=DockingExperienceStage::Docked)return false;s.stage=DockingExperienceStage::Undocking;s.progress=1.0;s.status="UNDOCKING / FOLLOW EXIT LIGHTS";return true;
}
} // namespace subspace
