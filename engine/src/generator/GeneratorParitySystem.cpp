#include "generator/GeneratorParitySystem.h"

#include "ships/ShipClassRoleSystem.h"

#include <sstream>

namespace subspace {
std::vector<GeneratorParityDescriptor> GeneratorParitySystem::Registry(){return {
{GeneratorDomain::Ship,"ship","Ship","ShipyardBuilderSystem / ShipPcgRuntimeClosureSystem",true,true,true,true,true},
{GeneratorDomain::Shuttle,"shuttle","Shuttle","ShipyardBuilderSystem / Shuttle profile",true,true,true,true,true},
{GeneratorDomain::Rover,"rover","Rover","UniversalConstructionSystem vehicle domain",true,true,false,true,true},
{GeneratorDomain::Station,"station","Station","StationWorkspaceSystem / StationDesignDnaSystem",true,true,false,true,true},
{GeneratorDomain::Interior,"interior","Interior","InteriorConstructionSystem / ShipInteriorCarvingSystem",true,true,false,true,true},
{GeneratorDomain::Prop,"prop","Prop / Fixture","ShipyardModelingSystem / AuthoringStandardsSystem",true,false,false,true,true},
{GeneratorDomain::Character,"character","Character","CharacterCustomizationSystem / CharacterAnimationLibrarySystem",true,false,false,true,true},
{GeneratorDomain::PlanetWorld,"planet_world","Planet / World","PlanetWorldEngineSystem",true,true,false,false,true},
{GeneratorDomain::SolarSystem,"solar_system","Solar System","SystemTopologySystem / OrbitalDynamicsSystem",true,false,false,false,true}
};}
const GeneratorParityDescriptor* GeneratorParitySystem::Find(const std::vector<GeneratorParityDescriptor>& r,GeneratorDomain d){for(const auto& x:r)if(x.domain==d)return &x;return nullptr;}
GeneratorParityValidation GeneratorParitySystem::Validate(const GeneratorParityRequest& q){GeneratorParityValidation v;const auto registry=Registry();const auto* d=Find(registry,q.domain);if(!d){v.errors.push_back("generator domain is not registered");return v;}if(q.seed==0)v.warnings.push_back("seed 0 is valid but discouraged for authored parity fixtures");if(q.profileId.empty())v.errors.push_back("generator request requires a profile id");if(d->supportsRole&&q.role.empty())v.warnings.push_back("generator role is empty; runtime default will be used");if(d->supportsClass){const auto e=ShipClassRoleSystem::Envelope(q.shipClass);if(e.maximumLengthMeters<=e.minimumLengthMeters)v.errors.push_back("ship class has an invalid physical envelope");}v.valid=v.errors.empty();return v;}
const char* GeneratorParitySystem::DomainId(GeneratorDomain d){switch(d){case GeneratorDomain::Ship:return"ship";case GeneratorDomain::Shuttle:return"shuttle";case GeneratorDomain::Rover:return"rover";case GeneratorDomain::Station:return"station";case GeneratorDomain::Interior:return"interior";case GeneratorDomain::Prop:return"prop";case GeneratorDomain::Character:return"character";case GeneratorDomain::PlanetWorld:return"planet_world";case GeneratorDomain::SolarSystem:return"solar_system";}return"ship";}
const char* GeneratorParitySystem::DomainName(GeneratorDomain d){switch(d){case GeneratorDomain::Ship:return"Ship";case GeneratorDomain::Shuttle:return"Shuttle";case GeneratorDomain::Rover:return"Rover";case GeneratorDomain::Station:return"Station";case GeneratorDomain::Interior:return"Interior";case GeneratorDomain::Prop:return"Prop / Fixture";case GeneratorDomain::Character:return"Character";case GeneratorDomain::PlanetWorld:return"Planet / World";case GeneratorDomain::SolarSystem:return"Solar System";}return"Ship";}
std::string GeneratorParitySystem::StableRequestIdentity(const GeneratorParityRequest& q){
    // FNV-1a over the canonical request fields. The same request must identify
    // the same generation transaction in Shipyard, PCG diagnostics and Blender.
    std::uint64_t h=1469598103934665603ull;
    auto feed=[&](const std::string& value){for(unsigned char c:value){h^=c;h*=1099511628211ull;}};
    feed(DomainId(q.domain));feed(std::to_string(q.seed));feed(q.profileId);feed(q.role);
    feed(ShipClassRoleSystem::ClassName(q.shipClass));feed(UniversalKitbashAuthority::SizeName(q.size));
    std::ostringstream out;out<<std::hex<<h;return out.str();
}
std::string GeneratorParitySystem::RequestJson(const GeneratorParityRequest& q){std::ostringstream o;o<<"{\n  \"schema\": \"subspace.generator-request.v1\",\n  \"domain\": \""<<DomainId(q.domain)<<"\",\n  \"seed\": "<<q.seed<<",\n  \"profileId\": \""<<q.profileId<<"\",\n  \"role\": \""<<q.role<<"\",\n  \"shipClass\": \""<<ShipClassRoleSystem::ClassName(q.shipClass)<<"\",\n  \"size\": \""<<UniversalKitbashAuthority::SizeName(q.size)<<"\"\n}\n";return o.str();}

} // namespace subspace
