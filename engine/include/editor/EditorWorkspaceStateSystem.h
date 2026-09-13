#pragma once

#include "editor/EditorDockSystem.h"
#include "editor/SubspaceEditorCore.h"

#include <string>
#include <unordered_map>

namespace subspace {

struct EditorWorkspacePreferences {
    EditorWorkspaceKind workspace = EditorWorkspaceKind::Shipyard;
    bool showAssetBrowser = true;
    bool showInspector = true;
    bool showOutliner = true;
    bool showBottomPanel = false;
    bool helpMode = true;
    float leftPanelWidth = 300.0f;
    float rightPanelWidth = 340.0f;
    float bottomPanelHeight = 180.0f;
    std::string activeBottomPanel = "validation";
};

class EditorWorkspaceStateSystem {
public:
    void Set(EditorWorkspacePreferences preferences);
    EditorWorkspacePreferences Get(EditorWorkspaceKind workspace) const;

    void SetDockWorkspace(EditorDockWorkspace workspace);
    EditorDockWorkspace GetDockWorkspace(EditorWorkspaceKind workspace) const;
    EditorDockWorkspace& MutableDockWorkspace(EditorWorkspaceKind workspace);

    void Reset(EditorWorkspaceKind workspace);
private:
    std::unordered_map<int, EditorWorkspacePreferences> state_;
    std::unordered_map<int, EditorDockWorkspace> dockState_;
};

} // namespace subspace
