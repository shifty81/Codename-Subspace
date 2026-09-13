#pragma once

#include "editor/SubspaceEditorCore.h"

#include <string>
#include <vector>

namespace subspace {

enum class EditorDockSplitAxis { Horizontal, Vertical };

struct EditorDockRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct EditorDockPanelDescriptor {
    std::string id;
    std::string title;
    std::string defaultLeafId;
    bool visible = false;
    bool canClose = true;
    bool canFloat = true;
    float minWidth = 160.0f;
    float minHeight = 100.0f;
};

struct EditorDockNode {
    std::string id;
    bool split = false;
    EditorDockSplitAxis axis = EditorDockSplitAxis::Horizontal;
    float ratio = 0.5f;
    std::string firstChildId;
    std::string secondChildId;
    std::vector<std::string> tabs;
    std::string activeTabId;
    bool collapsed = false;
};

struct EditorFloatingDockPanel {
    std::string panelId;
    EditorDockRect rect{80.0f, 80.0f, 420.0f, 320.0f};
};

struct EditorDockWorkspace {
    EditorWorkspaceKind workspace = EditorWorkspaceKind::Shipyard;
    std::string rootNodeId;
    std::vector<EditorDockPanelDescriptor> panels;
    std::vector<EditorDockNode> nodes;
    std::vector<EditorFloatingDockPanel> floatingPanels;
};

struct EditorDockPanelLayout {
    std::string panelId;
    std::string leafId;
    EditorDockRect rect{};
    bool visible = false;
    bool active = false;
    bool floating = false;
};

class EditorDockSystem {
public:
    static EditorDockWorkspace CreateDefault(EditorWorkspaceKind workspace = EditorWorkspaceKind::Shipyard);

    static EditorDockPanelDescriptor* FindPanel(EditorDockWorkspace& workspace, const std::string& panelId);
    static const EditorDockPanelDescriptor* FindPanel(const EditorDockWorkspace& workspace, const std::string& panelId);
    static EditorDockNode* FindNode(EditorDockWorkspace& workspace, const std::string& nodeId);
    static const EditorDockNode* FindNode(const EditorDockWorkspace& workspace, const std::string& nodeId);

    static bool RegisterPanel(EditorDockWorkspace& workspace, EditorDockPanelDescriptor panel);
    static bool OpenPanel(EditorDockWorkspace& workspace, const std::string& panelId);
    static bool ClosePanel(EditorDockWorkspace& workspace, const std::string& panelId);
    static bool TogglePanel(EditorDockWorkspace& workspace, const std::string& panelId);
    static bool ActivatePanel(EditorDockWorkspace& workspace, const std::string& panelId);
    static bool MovePanel(EditorDockWorkspace& workspace, const std::string& panelId, const std::string& targetLeafId, bool activate = true);
    static bool SplitLeaf(EditorDockWorkspace& workspace,
                          const std::string& leafId,
                          const std::string& newLeafId,
                          EditorDockSplitAxis axis,
                          float ratio,
                          const std::string& panelId,
                          bool placeNewAfter = true);
    static bool FloatPanel(EditorDockWorkspace& workspace, const std::string& panelId, EditorDockRect rect);
    static bool DockPanel(EditorDockWorkspace& workspace, const std::string& panelId, const std::string& targetLeafId, bool activate = true);

    static std::string LeafForPanel(const EditorDockWorkspace& workspace, const std::string& panelId);
    static bool IsFloating(const EditorDockWorkspace& workspace, const std::string& panelId);
    static std::vector<std::string> OpenPanelIds(const EditorDockWorkspace& workspace);
    static std::vector<EditorDockPanelLayout> Materialize(const EditorDockWorkspace& workspace, int width, int height, float topInset = 42.0f);
    static bool Validate(const EditorDockWorkspace& workspace, std::string* error = nullptr);
};

} // namespace subspace
