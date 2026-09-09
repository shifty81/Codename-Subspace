#include "navigation/AnomalousSpaceSystem.h"
#include <algorithm>
namespace subspace {
std::uint64_t AnomalousSpaceSystem::Open(AnomalousSpaceType t,const std::string& d,double s,double h,double m,double hazard){if(d.empty()||s<=0||h<=0||m<=0)return 0;auto id=nextId_++;pockets_[id]={id,t,d,std::clamp(s,0.0,1.0),h,m,0,std::clamp(hazard,0.0,1.0),false};return id;}
bool AnomalousSpaceSystem::Traverse(std::uint64_t id,double mass){auto it=pockets_.find(id);if(it==pockets_.end()||it->second.collapsed||mass<=0||it->second.massUsed+mass>it->second.massLimit)return false;it->second.massUsed+=mass;it->second.stability=std::max(0.0,it->second.stability-(mass/it->second.massLimit)*0.25);if(it->second.stability<=0)it->second.collapsed=true;return true;}
void AnomalousSpaceSystem::Tick(double hours){if(hours<=0)return;for(auto& kv:pockets_){auto& p=kv.second;if(p.collapsed)continue;p.remainingHours-=hours;p.stability=std::max(0.0,p.stability-hours*0.005);if(p.remainingHours<=0||p.stability<=0)p.collapsed=true;}}
const AnomalousSpacePocket* AnomalousSpaceSystem::Get(std::uint64_t id) const {auto it=pockets_.find(id);return it==pockets_.end()?nullptr:&it->second;}
}
