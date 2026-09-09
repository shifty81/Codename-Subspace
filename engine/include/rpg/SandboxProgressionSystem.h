#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class SandboxCareer { Combat, Mining, Salvage, Exploration, Trade, Industry, Logistics, FleetCommand, PlanetaryDevelopment, StationBuilding };
struct SandboxProgressionState { int ownedShips=1; int hiredCaptains=0; int stations=0; int planetaryColonies=0; int systemsOperating=1; double corporationAssets=0; std::vector<SandboxCareer> activeCareers; };
struct SandboxMilestone { std::string id; std::string title; bool achieved=false; };
class SandboxProgressionSystem {
public:
    std::vector<SandboxMilestone> Evaluate(const SandboxProgressionState& state) const;
    bool RequiresLinearCampaign() const { return false; }
};

} // namespace subspace
