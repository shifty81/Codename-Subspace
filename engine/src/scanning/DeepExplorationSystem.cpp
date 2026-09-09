#include "scanning/DeepExplorationSystem.h"
#include <algorithm>
namespace subspace {
bool DeepExplorationSystem::Register(const DeepExplorationSite& s){if(s.id.empty()||s.scanDifficulty<=0)return false;sites_[s.id]=s;return true;}
bool DeepExplorationSystem::Detect(const std::string& id){auto it=sites_.find(id);if(it==sites_.end())return false;if(it->second.state==DeepSiteState::Unknown)it->second.state=DeepSiteState::Detected;return true;}
double DeepExplorationSystem::Scan(const std::string& id,double strength,double seconds){auto it=sites_.find(id);if(it==sites_.end()||it->second.state==DeepSiteState::Unknown||strength<=0||seconds<=0)return 0;auto& s=it->second;s.scanProgress=std::min(1.0,s.scanProgress+(strength/s.scanDifficulty)*seconds*0.1);if(s.scanProgress>=1)s.state=DeepSiteState::Resolved;return s.scanProgress;}
bool DeepExplorationSystem::Exploit(const std::string& id){auto it=sites_.find(id);if(it==sites_.end()||it->second.state!=DeepSiteState::Resolved)return false;it->second.state=DeepSiteState::Exploited;return true;}
const DeepExplorationSite* DeepExplorationSystem::Get(const std::string& id) const {auto it=sites_.find(id);return it==sites_.end()?nullptr:&it->second;}
}
