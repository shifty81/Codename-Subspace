#include "procedural/ShipyardPcgStudioSystem.h"
#include "ships/ShipPcgRuntimeClosureSystem.h"
#include "ships/ShipFunctionalCoreSystem.h"

#include <sstream>

namespace subspace {

ShipyardPcgStudioReport ShipyardPcgStudioSystem::AuditCandidate(const std::vector<ShipyardModuleRecord>& catalog,
                                                                const ProceduralShipVisualRecipe& recipe,
                                                                const ShipyardPcgStudioRequest& request){
    ShipyardPcgStudioReport out;
    if(recipe.modules.empty())out.errors.push_back("Candidate contains no modules");
    auto runtime=ShipPcgRuntimeClosureSystem::EvaluateCandidate(catalog,recipe,false,true);
    if(runtime.accepted){out.decisions.push_back({"ship","SPATIAL_CERTIFICATION","Whole-ship spatial/propulsion authority accepted the candidate",true});}
    else{for(const auto&m:runtime.messages)out.errors.push_back(m);out.decisions.push_back({"ship","SPATIAL_CERTIFICATION","Whole-ship spatial/propulsion authority rejected the candidate",false});}
    const auto core=ShipFunctionalCoreSystem::Validate(catalog,recipe,true);
    for(const auto&m:core.messages)out.warnings.push_back(m);
    out.decisions.push_back({"ship","FUNCTIONAL_CORE",core.valid?"Required functional capability set is present":"One or more required functional capabilities need fitting",core.valid});
    if(request.factionId.empty())out.warnings.push_back("PCG request has no faction DesignDNA authority");
    if(request.hullFamilyIndex<1||request.hullFamilyIndex>4)out.errors.push_back("Hull family index must be 1..4");
    out.valid=out.errors.empty();return out;
}

std::uint32_t ShipyardPcgStudioSystem::RerollSeed(ShipyardPcgStudioState& state){++state.rerollCounter;std::uint32_t x=state.request.seed+0x9E3779B9u*state.rerollCounter;x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;state.request.seed=x;state.status="PCG seed rerolled to "+std::to_string(x);return x;}
std::vector<std::string> ShipyardPcgStudioSystem::EnabledOverlayNames(const ShipyardPcgOverlayState&o){std::vector<std::string>r;if(o.structuralSkeleton)r.push_back("STRUCTURAL");if(o.occupancy)r.push_back("OCCUPANCY");if(o.sockets)r.push_back("SOCKETS");if(o.propulsion)r.push_back("PROPULSION");if(o.exhaust)r.push_back("EXHAUST");if(o.commandExposure)r.push_back("COMMAND EXPOSURE");if(o.detailDensity)r.push_back("DETAIL DENSITY");if(o.functionalCore)r.push_back("FUNCTIONAL CORE");return r;}
std::string ShipyardPcgStudioSystem::ExplainRequest(const ShipyardPcgStudioRequest&r){std::ostringstream o;o<<"Faction "<<r.factionId<<" / "<<ShipClassRoleSystem::ClassName(r.shipClass)<<" / Hull Family "<<r.hullFamilyIndex<<" / "<<r.role<<" / Seed "<<r.seed;return o.str();}

} // namespace subspace
