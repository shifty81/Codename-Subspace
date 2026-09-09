#include "scanning/ExplorationDiscoverySystem.h"

#include <algorithm>

namespace subspace {
std::uint64_t ExplorationDiscoverySystem::Add(const ExplorationSignature&s){auto x=s;x.id=nextId_++;signatures_[x.id]=x;progress_[x.id]=x.resolved?1.0:0.0;return x.id;}
double ExplorationDiscoverySystem::Scan(std::uint64_t id,double strength,double seconds){auto it=signatures_.find(id);if(it==signatures_.end())return 0;double& p=progress_[id];p=std::clamp(p+std::max(0.0,strength)*std::max(0.0,seconds)*(0.06/std::max(.1,it->second.difficulty)),0.0,1.0);if(p>=1.0)it->second.resolved=true;return p;}
bool ExplorationDiscoverySystem::Bookmark(std::uint64_t id){auto it=signatures_.find(id);if(it==signatures_.end()||!it->second.resolved)return false;it->second.bookmarked=true;return true;}
std::vector<ExplorationSignature> ExplorationDiscoverySystem::Resolved()const{std::vector<ExplorationSignature>o;for(const auto&kv:signatures_)if(kv.second.resolved)o.push_back(kv.second);return o;}
const ExplorationSignature* ExplorationDiscoverySystem::Get(std::uint64_t id)const{auto it=signatures_.find(id);return it==signatures_.end()?nullptr:&it->second;}
} // namespace subspace
