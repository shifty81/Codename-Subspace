#include "station/StationPcgExemplarSystem.h"
#include <algorithm>
namespace subspace {
bool StationPcgExemplarSystem::Add(StationDesignExemplar e){if(e.name.empty()||e.dna.moduleCount<=0)return false;auto it=std::find_if(exemplars_.begin(),exemplars_.end(),[&](const auto&x){return x.name==e.name;});if(it!=exemplars_.end())*it=std::move(e);else exemplars_.push_back(std::move(e));return true;}
bool StationPcgExemplarSystem::Remove(const std::string&name){auto it=std::remove_if(exemplars_.begin(),exemplars_.end(),[&](const auto&e){return e.name==name;});if(it==exemplars_.end())return false;exemplars_.erase(it,exemplars_.end());return true;}
const StationDesignExemplar* StationPcgExemplarSystem::Find(const std::string&name)const{auto it=std::find_if(exemplars_.begin(),exemplars_.end(),[&](const auto&e){return e.name==name;});return it==exemplars_.end()?nullptr:&*it;}
std::vector<StationDesignExemplar> StationPcgExemplarSystem::ForArchetype(StationArchetype a)const{std::vector<StationDesignExemplar>o;for(const auto&e:exemplars_)if(e.dna.archetype==a)o.push_back(e);return o;}
StationDesignFamily StationPcgExemplarSystem::Compile(const std::string&id,StationArchetype a)const{return StationDesignDnaSystem::CompileFamily(id,a,ForArchetype(a));}
} // namespace subspace
