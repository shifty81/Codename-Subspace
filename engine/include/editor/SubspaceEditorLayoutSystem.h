#pragma once

#include "editor/EditorDockSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class EditorPanelSlot { AssetBrowser, Viewport, Inspector, Outliner, Validation, Console, Pcg, Jobs, History, Selection, Properties, Diagnostics, RuntimeDiff, AI, Custom };
struct EditorPanelLayout {
    EditorPanelSlot slot = EditorPanelSlot::Viewport;
    std::string id;
    float x = 0, y = 0, width = 0, height = 0;
    bool visible = true;
    bool collapsible = true;
    bool active = true;
    bool floating = false;
    std::string dockLeafId;
};
struct EditorWorkspaceLayout {
    int viewportWidth = 0, viewportHeight = 0;
    float topBarHeight = 42;
    float leftWidth = 300;
    float rightWidth = 340;
    float bottomHeight = 180;
    std::vector<EditorPanelLayout> panels;
};
class SubspaceEditorLayoutSystem {
public:
    // Compatibility entry point. It now delegates to the canonical dock model.
    static EditorWorkspaceLayout Build(int width, int height, bool showOutliner = true, bool showBottom = false);
    static EditorWorkspaceLayout BuildDocked(const EditorDockWorkspace& dock, int width, int height, float topBarHeight = 42.0f);
    static bool Validate(const EditorWorkspaceLayout& layout, std::string* error = nullptr);
};

} // namespace subspace
