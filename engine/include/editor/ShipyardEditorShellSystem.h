#pragma once

#include "editor/EditorDockSystem.h"
#include "editor/SubspaceEditorCore.h"

#include <string>
#include <vector>

namespace subspace {

/// One native Blender-like editor application. "Shipyard" is the editor/engine
/// identity; Ship, Character, World, Interior, Materials, Animation, PCG, VFX,
/// Audio, Logic and Diagnostics are workspaces inside the same shell.
class ShipyardEditorShellSystem {
public:
    static EditorWorkspaceRegistry BuildWorkspaceRegistry();
    static std::vector<EditorWorkspaceKind> PrimaryWorkspaceOrder();
    static bool IsPrimaryWorkspace(EditorWorkspaceKind workspace);
    static const char* WorkspaceName(EditorWorkspaceKind workspace);
    static EditorDockWorkspace CreateWorkspaceDock(EditorWorkspaceKind workspace);
};

} // namespace subspace
