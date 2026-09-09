#include "fleet/CarrierOperationsSystem.h"
#include <algorithm>
namespace subspace {
bool CarrierOperationsSystem::Register(const NestedCraft& c){if(c.id.empty()||static_cast<int>(craft_.size())>=capacity_)return false;craft_[c.id]=c;return true;}
int CarrierOperationsSystem::DeployedCount() const {int n=0;for(auto& kv:craft_)if(kv.second.state==NestedCraftState::Deployed||kv.second.state==NestedCraftState::Launching||kv.second.state==NestedCraftState::Recovering)n++;return n;}
bool CarrierOperationsSystem::Launch(const std::string& id){auto it=craft_.find(id);if(it==craft_.end()||it->second.state!=NestedCraftState::Stored||it->second.hull<=0||it->second.fuel<=0)return false;it->second.state=NestedCraftState::Deployed;return true;}
bool CarrierOperationsSystem::Recover(const std::string& id){auto it=craft_.find(id);if(it==craft_.end()||it->second.state!=NestedCraftState::Deployed)return false;it->second.state=NestedCraftState::Stored;return true;}
bool CarrierOperationsSystem::Service(const std::string& id,double hull,double ammo,double fuel){auto it=craft_.find(id);if(it==craft_.end()||it->second.state!=NestedCraftState::Stored)return false;it->second.hull=std::clamp(it->second.hull+hull,0.0,100.0);it->second.ammo=std::clamp(it->second.ammo+ammo,0.0,100.0);it->second.fuel=std::clamp(it->second.fuel+fuel,0.0,100.0);return true;}
const NestedCraft* CarrierOperationsSystem::Get(const std::string& id) const {auto it=craft_.find(id);return it==craft_.end()?nullptr:&it->second;}
}
