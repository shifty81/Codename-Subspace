#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct StoryDiscovery { std::uint64_t id=0; std::string title; std::string requiredCapability; bool discovered=false; bool resolved=false; bool mandatory=false; std::string worldStateEffect; };
class StorySpineSystem {
public:
    void SetStoryEnabled(bool enabled){enabled_=enabled;}
    bool IsStoryEnabled()const{return enabled_;}
    std::uint64_t Add(const StoryDiscovery& discovery);
    bool Discover(std::uint64_t id);
    bool Resolve(std::uint64_t id,const std::vector<std::string>& capabilities);
    std::vector<StoryDiscovery> Available()const;
    bool BlocksSandbox()const{return false;}
private:
    bool enabled_=true;std::uint64_t nextId_=1;std::unordered_map<std::uint64_t,StoryDiscovery> discoveries_;
};

} // namespace subspace
