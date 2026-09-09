#pragma once

#include "economy/LocalEconomySystem.h"
#include "runtime/SimulationLodSystem.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct LivingRegionState { std::string id;SimulationLod lod=SimulationLod::Aggregate;LocalEconomyState economy;std::vector<NpcEconomicAgent> corporations;double threat=0.2;double prosperity=0.5;std::uint64_t simulatedHours=0; };
struct LivingSandboxEvent { std::string regionId;std::string description;double severity=0; };

class LivingSandboxSystem {
public:
    void Advance(LivingRegionState& region,double hours) const;
    std::vector<LivingSandboxEvent> EvaluateEvents(const LivingRegionState& region) const;
    bool EconomyIsCausal(const LivingRegionState& region) const;
private:
    SimulationLodSystem lod_;LocalEconomySystem local_;
};

} // namespace subspace
