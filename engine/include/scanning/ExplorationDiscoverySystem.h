#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class ExplorationSignatureType { Combat, Data, Relic, Gas, Mining, Salvage, Research, Distress, HiddenStation, SubspaceTear, Story };
struct ExplorationSignature { std::uint64_t id=0; ExplorationSignatureType type=ExplorationSignatureType::Salvage; std::string name; double signalStrength=.25; double difficulty=.5; bool resolved=false; bool bookmarked=false; std::uint64_t escalationId=0; };
class ExplorationDiscoverySystem {
public:
    std::uint64_t Add(const ExplorationSignature& signature);
    double Scan(std::uint64_t id,double probeStrength,double seconds);
    bool Bookmark(std::uint64_t id);
    std::vector<ExplorationSignature> Resolved() const;
    const ExplorationSignature* Get(std::uint64_t id) const;
private:
    std::uint64_t nextId_=1;
    std::unordered_map<std::uint64_t,ExplorationSignature> signatures_;
    std::unordered_map<std::uint64_t,double> progress_;
};

} // namespace subspace
