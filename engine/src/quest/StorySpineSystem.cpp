#include "quest/StorySpineSystem.h"

#include <algorithm>
namespace subspace {
std::uint64_t StorySpineSystem::Add(const StoryDiscovery&d){auto x=d;x.id=nextId_++;discoveries_[x.id]=x;return x.id;}
bool StorySpineSystem::Discover(std::uint64_t id){auto it=discoveries_.find(id);if(it==discoveries_.end()||!enabled_)return false;it->second.discovered=true;return true;}
bool StorySpineSystem::Resolve(std::uint64_t id,const std::vector<std::string>&caps){auto it=discoveries_.find(id);if(it==discoveries_.end()||!it->second.discovered)return false;if(!it->second.requiredCapability.empty()&&std::find(caps.begin(),caps.end(),it->second.requiredCapability)==caps.end())return false;it->second.resolved=true;return true;}
std::vector<StoryDiscovery> StorySpineSystem::Available()const{std::vector<StoryDiscovery>o;if(!enabled_)return o;for(const auto&kv:discoveries_)if(kv.second.discovered&&!kv.second.resolved)o.push_back(kv.second);return o;}
} // namespace subspace
