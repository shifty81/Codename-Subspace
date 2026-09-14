#pragma once

#include "ship_editor/ShipyardWorkspaceSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipyardPanelDockHint {
    Left,
    RightTop,
    RightBottom,
    Bottom,
    Floating,
    Hidden
};

struct ShipyardPanelDescriptor {
    std::string id;
    std::string title;
    ShipyardPanelDockHint defaultDock = ShipyardPanelDockHint::Hidden;
    float minimumWidth = 220.0f;
    float minimumHeight = 160.0f;
    bool permanentShell = false;
    bool advanced = false;
    std::vector<ShipyardWorkspaceMode> workspaces;
};

struct ShipyardLayoutPreset {
    std::string id;
    std::string title;
    std::vector<std::string> visiblePanels;
    bool advanced = false;
};

struct ShipyardToolDescriptor {
    std::string commandId;
    std::string label;
    std::string shortcut;
    std::string tooltip;
};

struct ShipyardMenuDescriptor {
    std::string id;
    std::string label;
    std::vector<std::string> commandIds;
};

struct ShipyardQuickActionDescriptor {
    std::string commandId;
    std::string label;
    std::string shortcut;
    bool advanced = false;
};

class ShipyardProfessionalUiSystem {
public:
    static std::vector<ShipyardPanelDescriptor> Panels();
    static std::vector<ShipyardLayoutPreset> LayoutPresets();
    static std::vector<ShipyardToolDescriptor> PrimaryTools();
    static std::vector<ShipyardMenuDescriptor> Menus();
    static std::vector<ShipyardQuickActionDescriptor> QuickActions();
    static std::vector<ShipyardWorkspaceMode> PrimaryWorkspaceStrip();
    static std::vector<ShipyardWorkspaceMode> AdvancedWorkspaceMenu();
    static bool PanelAppliesTo(const ShipyardPanelDescriptor& panel, ShipyardWorkspaceMode workspace);
};

} // namespace subspace
