#pragma once
#include <string>
#include <vector>
namespace subspace {
struct SandboxAcceptanceIIState { bool combatOperational=false; bool damageConsequences=false; bool repairsConsumeResources=false; bool ewarOperational=false; bool dronesOperational=false; bool fleetDoctrineOperational=false; bool dynamicFactions=false; bool explorationOperational=false; bool anomalousSpace=false; bool hazardsOperational=false; bool materialEconomy=false; bool logisticsAutomated=false; bool stationEconomy=false; bool orbitalInfrastructure=false; bool capitalConstruction=false; bool carrierOperations=false; bool corporationProgression=false; bool persistentUniverse=false; };
struct SandboxAcceptanceIIReport { bool passed=false; int satisfied=0; int required=18; std::vector<std::string> missing; };
class SandboxAcceptanceIISystem { public: SandboxAcceptanceIIReport Evaluate(const SandboxAcceptanceIIState& state) const; };
}
