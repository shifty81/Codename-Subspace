#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
enum class DeepSiteType { Derelict, AncientStructure, HiddenResource, DataVault, WreckField, ResearchRuin, UnknownSignal };
enum class DeepSiteState { Unknown, Detected, Resolved, Exploited };
struct DeepExplorationSite { std::string id; DeepSiteType type=DeepSiteType::UnknownSignal; DeepSiteState state=DeepSiteState::Unknown; double scanDifficulty=1; double scanProgress=0; double danger=0; double value=0; std::vector<std::string> discoveries; };
class DeepExplorationSystem {
public:
 bool Register(const DeepExplorationSite& site);
 bool Detect(const std::string& id);
 double Scan(const std::string& id,double probeStrength,double seconds);
 bool Exploit(const std::string& id);
 const DeepExplorationSite* Get(const std::string& id) const;
private:std::unordered_map<std::string,DeepExplorationSite> sites_;
};
}
