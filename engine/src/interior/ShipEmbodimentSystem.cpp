#include "interior/ShipEmbodimentSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kHalfPi=1.57079632679489661923f;
}

bool ShipEmbodimentSystem::ExitCockpit(std::uint64_t shipId) {
    if (mode_ != ShipEmbodimentMode::CockpitControl || shipId == 0) return false;
    avatar_.shipId = shipId; avatar_.localPosition = {0.0f, 1.45f, 0.0f}; avatar_.deck = 0;
    avatar_.lookPitchRadians=0.0f; avatar_.facingRadians=avatar_.lookYawRadians;
    mode_ = ShipEmbodimentMode::InteriorOnFoot; return true;
}
bool ShipEmbodimentSystem::TakeControls() {
    if (mode_ != ShipEmbodimentMode::InteriorOnFoot || avatar_.shipId == 0) return false;
    if (std::sqrt(avatar_.localPosition.x*avatar_.localPosition.x + (avatar_.localPosition.y-1.45f)*(avatar_.localPosition.y-1.45f)) > 0.35f) return false;
    mode_ = ShipEmbodimentMode::CockpitControl; return true;
}
bool ShipEmbodimentSystem::EnterDockedHangar(std::uint64_t shipId) { if(shipId==0)return false;avatar_.shipId=shipId;mode_=ShipEmbodimentMode::DockedHangar;return true; }
bool ShipEmbodimentSystem::BoardInterior(std::uint64_t shipId) { if(shipId==0)return false;avatar_.shipId=shipId;avatar_.localPosition={0.0f,-1.55f,0.0f};avatar_.lookPitchRadians=0.0f;mode_=ShipEmbodimentMode::InteriorOnFoot;return true; }
void ShipEmbodimentSystem::SetInspection(bool enabled) { if(mode_==ShipEmbodimentMode::InteriorOnFoot||mode_==ShipEmbodimentMode::DockedHangar)return;mode_=enabled?ShipEmbodimentMode::CutawayInspection:ShipEmbodimentMode::CockpitControl; }
void ShipEmbodimentSystem::Look(float yawDeltaRadians,float pitchDeltaRadians){
    if(mode_!=ShipEmbodimentMode::InteriorOnFoot)return;
    avatar_.lookYawRadians+=yawDeltaRadians;
    avatar_.lookPitchRadians=std::clamp(avatar_.lookPitchRadians+pitchDeltaRadians,-kHalfPi+0.035f,kHalfPi-0.035f);
    avatar_.facingRadians=avatar_.lookYawRadians;
}
void ShipEmbodimentSystem::Move(float forward,float strafe,double seconds) {
    if(mode_!=ShipEmbodimentMode::InteriorOnFoot||seconds<=0)return;
    const float dt=static_cast<float>(std::min(0.10,seconds));
    const float f=std::clamp(forward,-1.0f,1.0f),s=std::clamp(strafe,-1.0f,1.0f);
    const float sy=std::sin(avatar_.lookYawRadians),cy=std::cos(avatar_.lookYawRadians);
    const Vector3 forwardAxis{-sy,cy,0.0f};
    const Vector3 rightAxis{cy,sy,0.0f};
    Vector3 delta=(forwardAxis*f+rightAxis*s)*(avatar_.moveSpeed*dt);
    avatar_.localPosition=avatar_.localPosition+delta;
    if(traversalBounds_.enabled){
        avatar_.localPosition.x=std::clamp(avatar_.localPosition.x,traversalBounds_.minimum.x,traversalBounds_.maximum.x);
        avatar_.localPosition.y=std::clamp(avatar_.localPosition.y,traversalBounds_.minimum.y,traversalBounds_.maximum.y);
        avatar_.localPosition.z=std::clamp(avatar_.localPosition.z,traversalBounds_.minimum.z,traversalBounds_.maximum.z);
    }
}
Vector3 ShipEmbodimentSystem::EyeLocalPosition() const {
    return avatar_.localPosition+Vector3{0.0f,0.0f,avatar_.eyeHeightMeters};
}
} // namespace subspace
