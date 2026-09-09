#include "ui/ShipActionHotbarSystem.h"
#include <algorithm>
namespace subspace {
std::vector<HotbarSlot> ShipActionHotbarSystem::BuildDefault(const std::vector<std::string>& c) const {std::vector<HotbarSlot> out;int s=1;auto has=[&](const char*x){return std::find(c.begin(),c.end(),x)!=c.end();};if(has("weapon"))out.push_back({s++,"primary","Primary Weapon",HotbarActionKind::Weapon,0,0,8,-1,true,true});if(has("mining"))out.push_back({s++,"mine","Mining Beam",HotbarActionKind::Mining,0,0,6,-1,true,true});if(has("scanner"))out.push_back({s++,"scan","Scanner Pulse",HotbarActionKind::Scanner,0,0,4,-1,true,false});if(has("repair"))out.push_back({s++,"repair","Repair System",HotbarActionKind::Repair,0,0,10,-1,true,false});if(has("drone"))out.push_back({s++,"drone","Deploy Drones",HotbarActionKind::Drone,0,0,5,-1,true,false});if(has("ewar"))out.push_back({s++,"ewar","Electronic Warfare",HotbarActionKind::Ewar,0,0,12,-1,true,true});return out;}
bool ShipActionHotbarSystem::CanActivate(const HotbarSlot&s,bool target,float power) const{return s.enabled&&s.cooldown<=0&&s.heat<1.0f&&(!s.requiresTarget||target)&&power>=s.powerCost&&(s.ammo!=0);}
} // namespace subspace
