#include "interior/ShipEmbodimentSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
bool ShipEmbodimentSystem::ExitCockpit(std::uint64_t shipId) {
    if (mode_ != ShipEmbodimentMode::CockpitControl || shipId == 0) return false;
    avatar_.shipId = shipId; avatar_.localPosition = {0.0f, 1.45f, 0.0f}; avatar_.deck = 0;
    mode_ = ShipEmbodimentMode::InteriorOnFoot; return true;
}
bool ShipEmbodimentSystem::TakeControls() {
    if (mode_ != ShipEmbodimentMode::InteriorOnFoot || avatar_.shipId == 0) return false;
    if (std::sqrt(avatar_.localPosition.x*avatar_.localPosition.x + (avatar_.localPosition.y-1.45f)*(avatar_.localPosition.y-1.45f)) > 0.35f) return false;
    mode_ = ShipEmbodimentMode::CockpitControl; return true;
}
bool ShipEmbodimentSystem::EnterDockedHangar(std::uint64_t shipId) { if(shipId==0)return false;avatar_.shipId=shipId;mode_=ShipEmbodimentMode::DockedHangar;return true; }
bool ShipEmbodimentSystem::BoardInterior(std::uint64_t shipId) { if(shipId==0)return false;avatar_.shipId=shipId;avatar_.localPosition={0.0f,-1.55f,0.0f};mode_=ShipEmbodimentMode::InteriorOnFoot;return true; }
void ShipEmbodimentSystem::SetInspection(bool enabled) { if(mode_==ShipEmbodimentMode::InteriorOnFoot||mode_==ShipEmbodimentMode::DockedHangar)return;mode_=enabled?ShipEmbodimentMode::CutawayInspection:ShipEmbodimentMode::CockpitControl; }
void ShipEmbodimentSystem::Move(float forward,float strafe,double seconds) {
    if(mode_!=ShipEmbodimentMode::InteriorOnFoot||seconds<=0)return;
    const float dt=static_cast<float>(std::min(0.10,seconds));
    avatar_.localPosition.x += std::clamp(strafe,-1.0f,1.0f)*avatar_.moveSpeed*dt;
    avatar_.localPosition.y += std::clamp(forward,-1.0f,1.0f)*avatar_.moveSpeed*dt;
    avatar_.localPosition.x = std::clamp(avatar_.localPosition.x,-1.75f,1.75f);
    avatar_.localPosition.y = std::clamp(avatar_.localPosition.y,-2.45f,2.15f);
    if(std::fabs(forward)+std::fabs(strafe)>0.01f)avatar_.facingRadians=std::atan2(-strafe,forward);
}
} // namespace subspace
