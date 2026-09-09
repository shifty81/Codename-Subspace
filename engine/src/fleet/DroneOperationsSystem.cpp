#include "fleet/DroneOperationsSystem.h"
#include <algorithm>
namespace subspace {
bool DroneOperationsSystem::Register(const DroneUnit& d){if(d.id.empty()||d.bandwidth<0||d.cargoCapacity<0)return false;drones_[d.id]=d;return true;}
int DroneOperationsSystem::BandwidthUsed() const {int n=0;for(auto& kv:drones_)if(kv.second.state==DroneState::Launching||kv.second.state==DroneState::Active||kv.second.state==DroneState::Returning)n+=kv.second.bandwidth;return n;}
bool DroneOperationsSystem::Launch(const std::string& id){auto it=drones_.find(id);if(it==drones_.end()||it->second.state!=DroneState::Stored||BandwidthUsed()+it->second.bandwidth>bandwidthLimit_)return false;it->second.state=DroneState::Active;return true;}
bool DroneOperationsSystem::Assign(const std::string& id,const std::string& target){auto it=drones_.find(id);if(it==drones_.end()||it->second.state!=DroneState::Active||target.empty())return false;it->second.targetId=target;it->second.taskProgress=0;return true;}
bool DroneOperationsSystem::Recall(const std::string& id){auto it=drones_.find(id);if(it==drones_.end()||it->second.state!=DroneState::Active)return false;it->second.state=DroneState::Returning;return true;}
void DroneOperationsSystem::Tick(double seconds){if(seconds<=0)return;for(auto& kv:drones_){auto& d=kv.second;if(d.state==DroneState::Active&&!d.targetId.empty()){d.taskProgress=std::min(1.0,d.taskProgress+seconds*0.1);if((d.role==DroneRole::Mining||d.role==DroneRole::Salvage||d.role==DroneRole::Cargo)&&d.cargoCapacity>0)d.cargo=std::min(d.cargoCapacity,d.cargo+seconds);}else if(d.state==DroneState::Returning){d.state=DroneState::Stored;d.targetId.clear();}}}
double DroneOperationsSystem::CollectCargo(const std::string& id){auto it=drones_.find(id);if(it==drones_.end()||it->second.state!=DroneState::Stored)return 0;double v=it->second.cargo;it->second.cargo=0;return v;}
const DroneUnit* DroneOperationsSystem::Get(const std::string& id) const {auto it=drones_.find(id);return it==drones_.end()?nullptr:&it->second;}
}
