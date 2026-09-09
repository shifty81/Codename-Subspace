#include "station/OrbitalInfrastructureSystem.h"
namespace subspace {
std::uint64_t OrbitalInfrastructureSystem::Build(OrbitalInfrastructureType t,const std::string& owner,const std::string& region,double power,double throughput){if(owner.empty()||region.empty()||power<0||throughput<0)return 0;auto id=nextId_++;infrastructure_[id]={id,t,owner,region,power,throughput,100,true};return id;}
bool OrbitalInfrastructureSystem::SetOnline(std::uint64_t id,bool online){auto it=infrastructure_.find(id);if(it==infrastructure_.end())return false;it->second.online=online;return true;}
double OrbitalInfrastructureSystem::TotalThroughput(const std::string& region) const {double total=0;for(auto& kv:infrastructure_)if(kv.second.parentRegionId==region&&kv.second.online&&kv.second.integrity>0)total+=kv.second.throughput;return total;}
const OrbitalInfrastructure* OrbitalInfrastructureSystem::Get(std::uint64_t id) const {auto it=infrastructure_.find(id);return it==infrastructure_.end()?nullptr:&it->second;}
}
