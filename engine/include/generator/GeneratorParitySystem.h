#pragma once

#include "content/UniversalKitbashAuthority.h"
#include "ships/ShipClassSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class GeneratorDomain { Ship,Shuttle,Rover,Station,Interior,Prop,Character,PlanetWorld,SolarSystem };
struct GeneratorParityDescriptor {
    GeneratorDomain domain=GeneratorDomain::Ship;
    std::string id;
    std::string displayName;
    std::string runtimeAuthority;
    bool supportsSeed=true;
    bool supportsRole=false;
    bool supportsClass=false;
    bool supportsSize=false;
    bool supportsLivePreview=true;
};
struct GeneratorParityRequest {
    GeneratorDomain domain=GeneratorDomain::Ship;
    std::uint32_t seed=1;
    std::string profileId="DEFAULT";
    std::string role="";
    ShipClass shipClass=ShipClass::Frigate;
    UniversalSizeClass size=UniversalSizeClass::S;
};
struct GeneratorParityValidation { bool valid=false; std::vector<std::string> errors,warnings; };

/// Registry shared by in-game Shipyard/dev mode and the Blender addon bridge.
/// Blender serializes requests to this contract; it must not maintain separate
/// procedural generation math for any domain.
class GeneratorParitySystem {
public:
    static std::vector<GeneratorParityDescriptor> Registry();
    static const GeneratorParityDescriptor* Find(const std::vector<GeneratorParityDescriptor>& registry,GeneratorDomain domain);
    static GeneratorParityValidation Validate(const GeneratorParityRequest& request);
    static const char* DomainId(GeneratorDomain domain);
    static const char* DomainName(GeneratorDomain domain);
    static std::string StableRequestIdentity(const GeneratorParityRequest& request);
    static std::string RequestJson(const GeneratorParityRequest& request);
};

} // namespace subspace
