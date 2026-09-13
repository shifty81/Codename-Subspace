#pragma once

#include "construction/AssemblyConstructionSystem.h"
#include "runtime/WorldSimulationAuthority.h"

#include <string>
#include <vector>

namespace subspace {

/// Binds Shipyard editing to the same persistent ship identity used by runtime
/// simulation. The working assembly is never a disconnected editor-only copy:
/// preview/commit/revert are explicit transactional states for one ship.
class ShipyardAssemblySession {
public:
    bool Begin(WorldSimulationAuthority& world,
               PersistentEntityId shipId,
               AssemblyDefinition assembly,
               std::string* error = nullptr);

    bool Apply(BuildCommand command, std::string* error = nullptr);
    bool Undo(std::string* error = nullptr) { return editor_.Undo(error); }
    bool Redo(std::string* error = nullptr) { return editor_.Redo(error); }

    AssemblyCompileSnapshot Preview(const std::vector<std::string>& requiredCapabilities = {}) const;
    bool Commit(const std::vector<std::string>& requiredCapabilities = {}, std::string* error = nullptr);
    bool Revert(std::string* error = nullptr);

    bool Active() const { return active_; }
    bool Dirty() const;
    PersistentEntityId ShipId() const { return shipId_; }
    const AssemblyDefinition& WorkingAssembly() const { return editor_.Current(); }
    const AssemblyDefinition& AcceptedAssembly() const { return accepted_; }
    std::string AcceptedFingerprint() const { return acceptedFingerprint_; }

private:
    WorldSimulationAuthority* world_ = nullptr;
    PersistentEntityId shipId_{};
    AssemblyDefinition accepted_{};
    std::string acceptedFingerprint_;
    AssemblyConstructionSystem editor_;
    bool active_ = false;
};

} // namespace subspace
