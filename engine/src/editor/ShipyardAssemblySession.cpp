#include "editor/ShipyardAssemblySession.h"

namespace subspace {

bool ShipyardAssemblySession::Begin(WorldSimulationAuthority& world,
                                    PersistentEntityId shipId,
                                    AssemblyDefinition assembly,
                                    std::string* error) {
    const auto* ship = world.Find(shipId);
    if (!ship || ship->header.kind != PersistentEntityKind::Ship) {
        if (error) *error = "Shipyard session requires a registered persistent Ship identity";
        return false;
    }
    if (assembly.assemblyId != shipId) {
        if (error) *error = "assembly stable identity does not match the docked ship";
        return false;
    }
    if (!editor_.Begin(assembly, error)) return false;
    const auto initial = editor_.Compile();
    if (!initial.valid) {
        if (error) *error = "initial ship assembly is not valid";
        return false;
    }
    world_ = &world;
    shipId_ = shipId;
    accepted_ = editor_.Current();
    acceptedFingerprint_ = initial.fingerprint;
    active_ = true;
    return true;
}

bool ShipyardAssemblySession::Apply(BuildCommand command, std::string* error) {
    if (!active_) {
        if (error) *error = "Shipyard session is not active";
        return false;
    }
    return editor_.Apply(std::move(command), error);
}

AssemblyCompileSnapshot ShipyardAssemblySession::Preview(const std::vector<std::string>& requiredCapabilities) const {
    if (!active_) return {};
    return editor_.Compile(requiredCapabilities);
}

bool ShipyardAssemblySession::Commit(const std::vector<std::string>& requiredCapabilities, std::string* error) {
    if (!active_ || !world_) {
        if (error) *error = "Shipyard session is not active";
        return false;
    }
    const auto snapshot = editor_.Compile(requiredCapabilities);
    if (!snapshot.valid) {
        if (error) *error = "Shipyard commit rejected by assembly validation";
        return false;
    }
    accepted_ = editor_.Current();
    acceptedFingerprint_ = snapshot.fingerprint;
    if (world_->MarkDirty(shipId_) == 0) {
        if (error) *error = "persistent ship disappeared before commit";
        return false;
    }
    // Rebase the edit journal at the accepted assembly; subsequent undo cannot
    // cross a commit boundary and mutate already-persisted state implicitly.
    if (!editor_.Begin(accepted_, error)) return false;
    return true;
}

bool ShipyardAssemblySession::Revert(std::string* error) {
    if (!active_) {
        if (error) *error = "Shipyard session is not active";
        return false;
    }
    return editor_.Begin(accepted_, error);
}

bool ShipyardAssemblySession::Dirty() const {
    if (!active_) return false;
    const auto snapshot = editor_.Compile();
    if (!snapshot.valid) return true;
    return snapshot.fingerprint != acceptedFingerprint_;
}

} // namespace subspace
