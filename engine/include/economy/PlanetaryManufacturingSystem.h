#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class PlanetFacilityType { ColonyControl, Extractor, DeepDrill, AtmosphericCollector, Power, Storage, Processor, Refinery, ComponentPlant, AdvancedPlant, LogisticsHub, SpaceElevator };
struct PlanetFacility { std::uint64_t id=0; PlanetFacilityType type=PlanetFacilityType::Storage; std::string recipe; double ratePerHour=0; double powerNet=0; double storage=0; };
struct PlanetLogisticsRoute { std::uint64_t from=0,to=0; std::string commodity; double capacityPerHour=0; };
struct PlanetaryManufacturingColony { std::uint64_t planetId=0; bool tetherSeedDeployed=false; double tetherConstructionProgress=0; bool elevatorOnline=false; double elevatorThroughputPerHour=0; std::vector<PlanetFacility> facilities; std::vector<PlanetLogisticsRoute> routes; std::unordered_map<std::string,double> inventory; };

class PlanetaryManufacturingSystem {
public:
    bool DeployTetherSeed(PlanetaryManufacturingColony& colony,bool surveyCertified) const;
    double DeliverElevatorMaterials(PlanetaryManufacturingColony& colony,double constructionUnits) const;
    bool AddFacility(PlanetaryManufacturingColony& colony,const PlanetFacility& facility) const;
    bool AddRoute(PlanetaryManufacturingColony& colony,const PlanetLogisticsRoute& route) const;
    double RouteThroughput(const PlanetaryManufacturingColony& colony,const std::string& commodity) const;
    double ExportToOrbit(PlanetaryManufacturingColony& colony,const std::string& commodity,double requested) const;
};

} // namespace subspace
