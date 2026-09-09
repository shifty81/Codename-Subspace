#include "combat/SectorCrisisSystem.h"

#include <algorithm>
namespace subspace {
std::uint64_t SectorCrisisSystem::Create(SectorCrisisType t,const std::string&n,double req){auto id=nextId_++;crises_[id]={id,t,n,std::max(1.0,req),0,false,{}};return id;}
bool SectorCrisisSystem::Contribute(std::uint64_t id,CrisisContribution c,double a){auto it=crises_.find(id);if(it==crises_.end()||it->second.completed||a<=0)return false;it->second.contribution[static_cast<int>(c)]+=a;it->second.progress=std::min(it->second.requiredProgress,it->second.progress+a);it->second.completed=it->second.progress>=it->second.requiredProgress;return true;}
const SectorCrisis* SectorCrisisSystem::Get(std::uint64_t id)const{auto it=crises_.find(id);return it==crises_.end()?nullptr:&it->second;}
} // namespace subspace
