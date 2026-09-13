#pragma once

#include "construction/ShipyardAssemblyBridgeSystem.h"
#include "editor/ShipyardAssemblySession.h"
#include "runtime/AssemblyRuntimeProductSystem.h"

#include <string>
#include <vector>

namespace subspace {

/// Transitional live-Shipyard bridge.  Existing certified kitbash catalogs and
/// recipes enter here, but all edits/preview/commit operations flow through the
/// canonical persistent-ship AssemblyDefinition/session authority.
class ShipyardCanonicalIntegrationSystem {
public:
    bool Begin(WorldSimulationAuthority& world,
               PersistentEntityId shipId,
               const std::vector<ShipyardModuleRecord>& catalog,
               const ProceduralShipVisualRecipe& recipe,
               std::string* error = nullptr);
    bool Apply(BuildCommand command,std::string* error=nullptr){return session_.Apply(std::move(command),error);}
    bool Undo(std::string* error=nullptr){return session_.Undo(error);}
    bool Redo(std::string* error=nullptr){return session_.Redo(error);}
    bool Commit(const std::vector<std::string>& requiredCapabilities={},std::string* error=nullptr){return session_.Commit(requiredCapabilities,error);}
    bool Revert(std::string* error=nullptr){return session_.Revert(error);}
    bool Active() const{return session_.Active();}
    bool Dirty() const{return session_.Dirty();}
    const AssemblyDefinition& WorkingAssembly() const{return session_.WorkingAssembly();}
    AssemblyRuntimeProducts PreviewProducts(const std::vector<std::string>& requiredCapabilities={}) const;
    const ShipyardAssemblyImportReport& ImportReport() const{return import_;}
private:
    std::vector<ShipyardModuleRecord> catalog_;
    ShipyardAssemblyImportReport import_{};
    ShipyardAssemblySession session_{};
};

} // namespace subspace
