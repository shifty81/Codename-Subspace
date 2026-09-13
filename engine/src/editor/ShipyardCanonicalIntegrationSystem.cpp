#include "editor/ShipyardCanonicalIntegrationSystem.h"

namespace subspace {

bool ShipyardCanonicalIntegrationSystem::Begin(WorldSimulationAuthority& world,
                                                PersistentEntityId shipId,
                                                const std::vector<ShipyardModuleRecord>& catalog,
                                                const ProceduralShipVisualRecipe& recipe,
                                                std::string* error){
    catalog_=catalog;import_=ShipyardAssemblyBridgeSystem::Import(catalog_,recipe,shipId);
    if(!import_.valid){if(error)*error=import_.errors.empty()?"Shipyard recipe import failed":import_.errors.front();return false;}
    return session_.Begin(world,shipId,import_.assembly,error);
}

AssemblyRuntimeProducts ShipyardCanonicalIntegrationSystem::PreviewProducts(const std::vector<std::string>& requiredCapabilities) const{
    if(!session_.Active())return {};
    return AssemblyRuntimeProductSystem::Compile(session_.WorkingAssembly(),catalog_,requiredCapabilities);
}

} // namespace subspace
