#pragma once

#include "procedural/GalaxyGenerator.h"
#include "economy/InfrastructureModuleSystem.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace subspace {

enum class PiInstallationKind { Extractor, Factory, Refinery, Storage, Power, Logistics, Research, Defense, Sensor, Shield, AtmosphericCollector };
struct HexCoord { int q=0,r=0; bool operator<(const HexCoord&o) const{return q!=o.q?q<o.q:r<o.r;} };
struct PiHexData { HexCoord coord; float resource=.5f; float hazard=.2f; float buildability=.8f; bool surveyed=false; bool perimeter=false; };
struct PiInstallation { std::uint64_t id=0; HexCoord hex; PiInstallationKind kind=PiInstallationKind::Extractor; PowerTechnology tech=PowerTechnology::Burner; double inputPerHour=0,outputPerHour=0,storage=0,power=0; bool active=true; };
struct PlanetaryIndustryState { PlanetIndustryRepresentation representation=PlanetIndustryRepresentation::SurfaceHexGrid; std::map<HexCoord,PiHexData> hexes; std::vector<PiInstallation> installations; double tetherStorage=0; double tetherThroughputPerHour=100; int environmentTier=0; };
struct PiValidation { bool valid=false; double production=0; double powerBalance=0; bool protectedEnough=true; std::vector<std::string> errors; };
class PlanetaryIndustrySystem {
public:
    PlanetaryIndustryState Generate(const PlanetData& planet,int radius,std::uint32_t seed) const;
    bool Place(PlanetaryIndustryState& state,PiInstallation installation) const;
    PiValidation Validate(const PlanetaryIndustryState& state,float planetHazard) const;
    double TransferToTether(PlanetaryIndustryState& state,double produced,double hours) const;
};

} // namespace subspace
