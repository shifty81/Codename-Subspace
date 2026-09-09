#include "research/BlueprintResearchSystem.h"

#include <algorithm>
namespace subspace {
bool BlueprintResearchSystem::Acquire(const BlueprintKnowledgeNative&b){if(b.id.empty())return false;auto c=b;c.owned=true;blueprints_[c.id]=c;return true;}
bool BlueprintResearchSystem::ResearchMaterial(const std::string&id){auto it=blueprints_.find(id);if(it==blueprints_.end()||!it->second.owned||it->second.materialResearch>=10)return false;++it->second.materialResearch;return true;}
bool BlueprintResearchSystem::ResearchTime(const std::string&id){auto it=blueprints_.find(id);if(it==blueprints_.end()||!it->second.owned||it->second.timeResearch>=10)return false;++it->second.timeResearch;return true;}
bool BlueprintResearchSystem::ReverseEngineer(const std::string&id,int tier){auto it=blueprints_.find(id);if(it==blueprints_.end()||tier<it->second.technologyTier)return false;it->second.reverseEngineered=true;return true;}
double BlueprintResearchSystem::MaterialMultiplier(const std::string&id)const{auto*p=Get(id);return p?std::max(.75,1.0-p->materialResearch*.02):1.0;}
double BlueprintResearchSystem::TimeMultiplier(const std::string&id)const{auto*p=Get(id);return p?std::max(.65,1.0-p->timeResearch*.03):1.0;}
const BlueprintKnowledgeNative* BlueprintResearchSystem::Get(const std::string&id)const{auto it=blueprints_.find(id);return it==blueprints_.end()?nullptr:&it->second;}
} // namespace subspace
