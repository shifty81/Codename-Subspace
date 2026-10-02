#pragma once

#include "procedural/GalaxyGenerator.h"
#include "economy/InfrastructureModuleSystem.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace subspace {

enum class PiInstallationKind {
    Extractor, Factory, Refinery, Storage, Power, Logistics, Research,
    Defense, Sensor, Shield, AtmosphericCollector
};

enum class PiClaimState { Unsurveyed = 0, Surveyed, Claimed, Developed };
enum class PiOverlayMode { Resources = 0, Ownership, Industry, Logistics, Power, Hazard };
enum class PiProjectionMode { Globe = 0, Sector };

struct HexCoord {
    int q=0,r=0;
    bool operator<(const HexCoord&o) const{return q!=o.q?q<o.q:r<o.r;}
    bool operator==(const HexCoord&o) const{return q==o.q&&r==o.r;}
};

struct PiSectorIdentity {
    std::string planetId;
    HexCoord coord{};
    std::string StableId() const;
};

struct PiHexData {
    HexCoord coord;
    float resource=.5f;
    float hazard=.2f;
    float buildability=.8f;
    bool surveyed=false; // compatibility mirror of claimState >= Surveyed
    bool perimeter=false;
    PiClaimState claimState=PiClaimState::Unsurveyed;
    std::uint64_t ownerId=0;
};

struct PiInstallation {
    std::uint64_t id=0;
    HexCoord hex;
    PiInstallationKind kind=PiInstallationKind::Extractor;
    PowerTechnology tech=PowerTechnology::Burner;
    double inputPerHour=0,outputPerHour=0,storage=0,power=0;
    bool active=true;
};

struct PlanetaryIndustryState {
    PlanetIndustryRepresentation representation=PlanetIndustryRepresentation::SurfaceHexGrid;
    std::string planetId;
    std::map<HexCoord,PiHexData> hexes;
    std::vector<PiInstallation> installations;
    double tetherStorage=0;
    double tetherThroughputPerHour=100;
    int environmentTier=0;
};

struct PiValidation {
    bool valid=false;
    double production=0;
    double powerBalance=0;
    bool protectedEnough=true;
    std::vector<std::string> errors;
};

struct PiSectorCommandResult {
    bool changed=false;
    PiClaimState before=PiClaimState::Unsurveyed;
    PiClaimState after=PiClaimState::Unsurveyed;
    std::string status;
};

class PlanetaryIndustrySystem {
public:
    PlanetaryIndustryState Generate(const PlanetData& planet,int radius,std::uint32_t seed) const;
    bool Place(PlanetaryIndustryState& state,PiInstallation installation) const;
    bool PlaceGoverned(PlanetaryIndustryState& state,PiInstallation installation,std::uint64_t ownerId) const;
    PiValidation Validate(const PlanetaryIndustryState& state,float planetHazard) const;
    double TransferToTether(PlanetaryIndustryState& state,double produced,double hours) const;

    bool SurveySector(PlanetaryIndustryState& state,HexCoord coord) const;
    bool CanClaim(const PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const;
    bool ClaimSector(PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const;
    bool DevelopSector(PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const;
    PiSectorCommandResult AdvanceSector(PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const;
    std::vector<HexCoord> ClaimFrontier(const PlanetaryIndustryState& state,std::uint64_t ownerId) const;
    PiSectorIdentity Identity(const PlanetaryIndustryState& state,HexCoord coord) const;

    static int HexDistance(HexCoord a,HexCoord b={});
    static bool Adjacent(HexCoord a,HexCoord b);
    static const char* ClaimStateName(PiClaimState state);
    static const char* OverlayName(PiOverlayMode mode);
    static const char* ProjectionName(PiProjectionMode mode);
};

} // namespace subspace
