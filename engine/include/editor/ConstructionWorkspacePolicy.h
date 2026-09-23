#pragma once

#include "ship_editor/ShipyardBuilderSystem.h"

namespace subspace {

// Public Studio treats assembly and geometry editing as one CONSTRUCT
// workspace.  Existing Build/Model enum values remain intact for save,
// command and undo compatibility while the shell presents them as submodes.
class ConstructionWorkspacePolicy {
public:
    static constexpr bool IsConstruct(ShipyardWorkspaceMode mode) noexcept {
        return mode==ShipyardWorkspaceMode::Build||mode==ShipyardWorkspaceMode::Model;
    }
    static constexpr const char* PublicWorkspaceName(ShipyardWorkspaceMode mode) noexcept {
        return IsConstruct(mode)?"CONSTRUCT":nullptr;
    }
    static constexpr const char* ConstructSubmodeName(ShipyardWorkspaceMode mode) noexcept {
        return mode==ShipyardWorkspaceMode::Model?"GEOMETRY":"ASSEMBLY";
    }
};

} // namespace subspace
