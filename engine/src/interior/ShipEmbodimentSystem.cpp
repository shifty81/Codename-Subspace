#include "interior/ShipEmbodimentSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kHalfPi=1.57079632679489661923f;
constexpr float kCommandSeatY=1.45f;
constexpr float kCommandSeatInteractionRadius=0.35f;
float Length2D(Vector3 v){return std::sqrt(v.x*v.x+v.y*v.y);}
Vector3 ClampMagnitude2D(Vector3 v,float maxLength){
    const float len=Length2D(v);if(len<=maxLength||len<=0.0001f)return v;
    const float scale=maxLength/len;return {v.x*scale,v.y*scale,0.0f};
}
Vector3 MoveToward2D(Vector3 current,Vector3 target,float maxDelta){
    const Vector3 d{target.x-current.x,target.y-current.y,0.0f};
    const float len=Length2D(d);if(len<=maxDelta||len<=0.0001f)return {target.x,target.y,0.0f};
    const float k=maxDelta/len;return {current.x+d.x*k,current.y+d.y*k,0.0f};
}
}

bool ShipEmbodimentSystem::ExitCockpit(std::uint64_t shipId) {
    if (mode_ != ShipEmbodimentMode::CockpitControl || shipId == 0) return false;
    avatar_.shipId = shipId; avatar_.localPosition = {0.0f, kCommandSeatY, 0.0f}; avatar_.deck = 0;
    avatar_.planarVelocity={};avatar_.grounded=true;avatar_.stance=InteriorAvatarStance::Standing;avatar_.sprinting=false;
    avatar_.lookPitchRadians=0.0f; avatar_.facingRadians=avatar_.lookYawRadians;
    avatar_.eyeHeightMeters=avatar_.standingEyeHeightMeters;avatar_.capsuleHeightMeters=avatar_.standingCapsuleHeightMeters;
    commandSeatInteractionReady_=true;commandSeatDeparted_=false;
    mode_ = ShipEmbodimentMode::InteriorOnFoot; return true;
}
bool ShipEmbodimentSystem::TakeControls() {
    if (mode_ != ShipEmbodimentMode::InteriorOnFoot || avatar_.shipId == 0) return false;
    const float seatDistance=std::sqrt(avatar_.localPosition.x*avatar_.localPosition.x +
        (avatar_.localPosition.y-kCommandSeatY)*(avatar_.localPosition.y-kCommandSeatY));
    if(!commandSeatInteractionReady_||seatDistance>kCommandSeatInteractionRadius)return false;
    StopLocomotion();mode_ = ShipEmbodimentMode::CockpitControl; return true;
}
bool ShipEmbodimentSystem::EnterDockedHangar(std::uint64_t shipId) { if(shipId==0)return false;avatar_.shipId=shipId;commandSeatInteractionReady_=false;commandSeatDeparted_=true;StopLocomotion();mode_=ShipEmbodimentMode::DockedHangar;return true; }
bool ShipEmbodimentSystem::BoardInterior(std::uint64_t shipId) { if(shipId==0)return false;avatar_.shipId=shipId;avatar_.localPosition={0.0f,-1.55f,0.0f};avatar_.lookPitchRadians=0.0f;avatar_.grounded=true;commandSeatInteractionReady_=false;commandSeatDeparted_=true;ConfigureLocomotion(false,false);StopLocomotion();mode_=ShipEmbodimentMode::InteriorOnFoot;return true; }
void ShipEmbodimentSystem::SetInspection(bool enabled) { if(mode_==ShipEmbodimentMode::InteriorOnFoot||mode_==ShipEmbodimentMode::DockedHangar)return;mode_=enabled?ShipEmbodimentMode::CutawayInspection:ShipEmbodimentMode::CockpitControl; }
void ShipEmbodimentSystem::RefreshCommandSeatInteraction(){
    if(mode_!=ShipEmbodimentMode::InteriorOnFoot)return;
    const float seatDistance=std::sqrt(avatar_.localPosition.x*avatar_.localPosition.x +
        (avatar_.localPosition.y-kCommandSeatY)*(avatar_.localPosition.y-kCommandSeatY));
    if(seatDistance>kCommandSeatInteractionRadius){commandSeatDeparted_=true;return;}
    if(commandSeatDeparted_)commandSeatInteractionReady_=true;
}
void ShipEmbodimentSystem::SetCertifiedFootPosition(Vector3 position){
    if(!IsOnFoot())return;avatar_.localPosition=position;RefreshCommandSeatInteraction();
}
void ShipEmbodimentSystem::Look(float yawDeltaRadians,float pitchDeltaRadians){
    if(mode_!=ShipEmbodimentMode::InteriorOnFoot)return;
    avatar_.lookYawRadians+=yawDeltaRadians;
    avatar_.lookPitchRadians=std::clamp(avatar_.lookPitchRadians+pitchDeltaRadians,-kHalfPi+0.035f,kHalfPi-0.035f);
    avatar_.facingRadians=avatar_.lookYawRadians;
}
void ShipEmbodimentSystem::ConfigureLocomotion(bool sprintRequested,bool crouchRequested){
    if(mode_!=ShipEmbodimentMode::InteriorOnFoot)return;
    avatar_.stance=crouchRequested?InteriorAvatarStance::Crouched:InteriorAvatarStance::Standing;
    avatar_.sprinting=sprintRequested&&!crouchRequested&&avatar_.stamina01>0.02f;
    avatar_.eyeHeightMeters=avatar_.stance==InteriorAvatarStance::Crouched?avatar_.crouchedEyeHeightMeters:avatar_.standingEyeHeightMeters;
    avatar_.capsuleHeightMeters=avatar_.stance==InteriorAvatarStance::Crouched?avatar_.crouchedCapsuleHeightMeters:avatar_.standingCapsuleHeightMeters;
}
void ShipEmbodimentSystem::StopLocomotion(){avatar_.planarVelocity={};avatar_.sprinting=false;}
void ShipEmbodimentSystem::Move(float forward,float strafe,double seconds) {
    if(mode_!=ShipEmbodimentMode::InteriorOnFoot||seconds<=0)return;
    const float dt=static_cast<float>(std::min(0.10,seconds));
    const float f=std::clamp(forward,-1.0f,1.0f),s=std::clamp(strafe,-1.0f,1.0f);
    const float sy=std::sin(avatar_.lookYawRadians),cy=std::cos(avatar_.lookYawRadians);
    const Vector3 forwardAxis{-sy,cy,0.0f};
    const Vector3 rightAxis{cy,sy,0.0f};
    Vector3 input=ClampMagnitude2D(forwardAxis*f+rightAxis*s,1.0f);
    if(Length2D(input)>0.001f)commandSeatInteractionReady_=false;

    float speed=avatar_.walkSpeedMetersPerSecond;
    if(avatar_.stance==InteriorAvatarStance::Crouched)speed=avatar_.crouchSpeedMetersPerSecond;
    else if(avatar_.sprinting)speed=avatar_.sprintSpeedMetersPerSecond;
    avatar_.moveSpeed=speed; // legacy/runtime HUD compatibility

    const Vector3 desired=input*speed;
    const bool accelerating=Length2D(desired)>0.001f;
    const float response=(accelerating?avatar_.accelerationMetersPerSecond2:avatar_.decelerationMetersPerSecond2)*dt;
    avatar_.planarVelocity=MoveToward2D(avatar_.planarVelocity,desired,response);
    avatar_.localPosition=avatar_.localPosition+avatar_.planarVelocity*dt;

    if(avatar_.sprinting&&accelerating)avatar_.stamina01=std::max(0.0f,avatar_.stamina01-dt*0.12f);
    else avatar_.stamina01=std::min(1.0f,avatar_.stamina01+dt*0.07f);
    if(avatar_.stamina01<=0.001f)avatar_.sprinting=false;

    if(traversalBounds_.enabled){
        avatar_.localPosition.x=std::clamp(avatar_.localPosition.x,traversalBounds_.minimum.x,traversalBounds_.maximum.x);
        avatar_.localPosition.y=std::clamp(avatar_.localPosition.y,traversalBounds_.minimum.y,traversalBounds_.maximum.y);
        avatar_.localPosition.z=std::clamp(avatar_.localPosition.z,traversalBounds_.minimum.z,traversalBounds_.maximum.z);
    }
    RefreshCommandSeatInteraction();
}
Vector3 ShipEmbodimentSystem::EyeLocalPosition() const {
    return avatar_.localPosition+Vector3{0.0f,0.0f,avatar_.eyeHeightMeters};
}
} // namespace subspace
