#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class SimulationLod { Full, Regional, Aggregate, Dormant };
enum class NpcEconomicAction { Mine, Refine, Manufacture, Haul, Consume, BuildShip, ReplaceLosses };
struct SimulationSubject { std::string id; double distanceMeters=0; bool playerPresent=false; bool important=false; };
struct NpcEconomicAgent { std::string id; std::unordered_map<std::string,double> inventory; double credits=0; double productionCapacity=1; double logisticsCapacity=1; int shipsLost=0; int shipsBuilt=0; };
class SimulationLodSystem {
public:
    SimulationLod Select(const SimulationSubject& subject) const;
    void AdvanceEconomy(NpcEconomicAgent& agent,NpcEconomicAction action,double hours) const;
};

} // namespace subspace
