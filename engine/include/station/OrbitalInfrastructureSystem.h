#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
namespace subspace {
enum class OrbitalInfrastructureType { ElevatorTerminal, RefineryComplex, Shipyard, ResearchArray, LogisticsDepot, DefensePlatform, Habitat, FleetDock };
struct OrbitalInfrastructure { std::uint64_t id=0; OrbitalInfrastructureType type=OrbitalInfrastructureType::LogisticsDepot; std::string ownerId; std::string parentRegionId; double powerDemand=0; double throughput=0; double integrity=100; bool online=true; };
class OrbitalInfrastructureSystem {
public:
 std::uint64_t Build(OrbitalInfrastructureType type,const std::string& owner,const std::string& region,double power,double throughput);
 bool SetOnline(std::uint64_t id,bool online);
 double TotalThroughput(const std::string& region) const;
 const OrbitalInfrastructure* Get(std::uint64_t id) const;
private:std::uint64_t nextId_=1;std::unordered_map<std::uint64_t,OrbitalInfrastructure> infrastructure_;
};
}
