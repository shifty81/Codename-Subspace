#include "combat/ShipFailureSystem.h"
#include <algorithm>
namespace subspace {
FailureState ShipFailureSystem::Resolve(double r){if(r<=0)return FailureState::Destroyed;if(r<=0.15)return FailureState::Disabled;if(r<=0.35)return FailureState::Failing;if(r<=0.70)return FailureState::Degraded;return FailureState::Nominal;}
bool ShipFailureSystem::Register(const ShipSubsystemState& s){if(s.id.empty()||s.maxIntegrity<=0)return false;auto c=s;c.integrity=std::clamp(c.integrity,0.0,c.maxIntegrity);c.state=Resolve(c.integrity/c.maxIntegrity);systems_[c.id]=c;return true;}
bool ShipFailureSystem::ApplyDamage(const std::string& id,double damage){auto it=systems_.find(id);if(it==systems_.end()||damage<=0)return false;it->second.integrity=std::max(0.0,it->second.integrity-damage);it->second.state=Resolve(it->second.integrity/it->second.maxIntegrity);return true;}
bool ShipFailureSystem::Restore(const std::string& id,double integrity){auto it=systems_.find(id);if(it==systems_.end()||integrity<=0)return false;it->second.integrity=std::min(it->second.maxIntegrity,it->second.integrity+integrity);it->second.state=Resolve(it->second.integrity/it->second.maxIntegrity);return true;}
double ShipFailureSystem::OperationalFraction(ShipSubsystemType type) const {double sum=0;int n=0;for(auto& kv:systems_)if(kv.second.type==type){sum+=kv.second.maxIntegrity>0?kv.second.integrity/kv.second.maxIntegrity:0;++n;}return n?sum/n:1.0;}
std::vector<std::string> ShipFailureSystem::CriticalFailures() const {std::vector<std::string> out;for(auto& kv:systems_)if(kv.second.critical&&(kv.second.state==FailureState::Disabled||kv.second.state==FailureState::Destroyed||kv.second.state==FailureState::Failing))out.push_back(kv.first);return out;}
const ShipSubsystemState* ShipFailureSystem::Get(const std::string& id) const {auto it=systems_.find(id);return it==systems_.end()?nullptr:&it->second;}
}
