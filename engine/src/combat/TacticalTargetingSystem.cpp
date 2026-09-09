#include "combat/TacticalTargetingSystem.h"
#include <algorithm>
namespace subspace {
namespace {bool Same(const TacticalTargetReference&a,const TacticalTargetReference&b){return a.IsValid()&&b.IsValid()&&a.kind==b.kind&&a.index==b.index&&a.id==b.id;}}
bool TacticalTargetingSystem::Request(TacticalTargetingState&s,const TacticalTargetReference&sel){if(!sel.IsValid())return false;for(auto&t:s.targets)if(Same(t.contact,sel)){for(auto&x:s.targets)x.active=false;t.active=true;return true;}if(s.targets.size()>=s.maxTargets)return false;for(auto&x:s.targets)x.active=false;s.targets.push_back({sel,0.0f,false,true});return true;}
void TacticalTargetingSystem::Tick(TacticalTargetingState&s,float dt,float acquire){if(dt<=0)return;for(auto&t:s.targets)if(!t.locked){t.lockProgress=std::min(1.0f,t.lockProgress+dt/std::max(.05f,acquire));if(t.lockProgress>=.999f){t.lockProgress=1.0f;t.locked=true;}}}
bool TacticalTargetingSystem::IsLocked(const TacticalTargetingState&s,const TacticalTargetReference&sel){for(const auto&t:s.targets)if(Same(t.contact,sel))return t.locked;return false;}
bool TacticalTargetingSystem::Activate(TacticalTargetingState&s,const TacticalTargetReference&sel){bool found=false;for(auto&t:s.targets){t.active=Same(t.contact,sel);found|=t.active;}return found;}
void TacticalTargetingSystem::ClearInvalid(TacticalTargetingState&s,const std::vector<std::string>&valid){s.targets.erase(std::remove_if(s.targets.begin(),s.targets.end(),[&](const auto&t){return std::find(valid.begin(),valid.end(),t.contact.id)==valid.end();}),s.targets.end());}
} // namespace subspace
