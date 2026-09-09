#pragma once

#include <string>
#include <unordered_map>

namespace subspace {

struct BlueprintKnowledgeNative { std::string id; bool owned=false; int materialResearch=0; int timeResearch=0; int technologyTier=1; bool reverseEngineered=false; };
class BlueprintResearchSystem {
public:
    bool Acquire(const BlueprintKnowledgeNative& blueprint);
    bool ResearchMaterial(const std::string& id);
    bool ResearchTime(const std::string& id);
    bool ReverseEngineer(const std::string& id,int recoveredTechTier);
    double MaterialMultiplier(const std::string& id) const;
    double TimeMultiplier(const std::string& id) const;
    const BlueprintKnowledgeNative* Get(const std::string& id) const;
private:
    std::unordered_map<std::string,BlueprintKnowledgeNative> blueprints_;
};

} // namespace subspace
