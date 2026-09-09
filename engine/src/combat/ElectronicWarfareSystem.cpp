#include "combat/ElectronicWarfareSystem.h"
#include <algorithm>
namespace subspace {
bool ElectronicWarfareSystem::Apply(const EWarEffect& e){if(e.id.empty()||e.strength<0||e.remainingSeconds<=0)return false;effects_[e.id]=e;return true;}
void ElectronicWarfareSystem::Tick(double seconds){if(seconds<=0)return;std::vector<std::string> gone;for(auto& kv:effects_){kv.second.remainingSeconds-=seconds;if(kv.second.remainingSeconds<=0)gone.push_back(kv.first);}for(auto& id:gone)effects_.erase(id);}
double ElectronicWarfareSystem::CombinedStrength(EWarEffectType type) const {double s=0;for(auto& kv:effects_)if(kv.second.type==type)s+=kv.second.strength;return std::clamp(s,0.0,1.0);}
double ElectronicWarfareSystem::CapabilityMultiplier(EWarEffectType type) const {return std::max(0.0,1.0-CombinedStrength(type));}
bool ElectronicWarfareSystem::Remove(const std::string& id){return effects_.erase(id)>0;}
}
