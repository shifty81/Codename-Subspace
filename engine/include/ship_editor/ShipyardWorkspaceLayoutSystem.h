#pragma once

#include "ship_editor/ShipyardProfessionalUiSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct ShipyardPanelLayoutState {
    std::string panelId;
    ShipyardPanelDockHint dock = ShipyardPanelDockHint::Hidden;
    bool visible = false;
    bool floating = false;
    float width = 0.0f;
    float height = 0.0f;
};

struct ShipyardWorkspaceLayoutState {
    std::string presetId = "BUILD";
    std::vector<ShipyardPanelLayoutState> panels;
    bool auxiliaryDocksCollapsed = false;
    std::string maximizedPanelId;
    bool dirty = false;
};

class ShipyardWorkspaceLayoutSystem {
public:
    static ShipyardWorkspaceLayoutState Default(const std::string& presetId = "BUILD");
    static bool ApplyPreset(ShipyardWorkspaceLayoutState& state, const std::string& presetId);
    static bool SetPanelVisible(ShipyardWorkspaceLayoutState& state, const std::string& panelId, bool visible);
    static bool ToggleMaximized(ShipyardWorkspaceLayoutState& state, const std::string& panelId);
    static void ToggleAuxiliaryDocks(ShipyardWorkspaceLayoutState& state);
    static bool ResetCurrent(ShipyardWorkspaceLayoutState& state);
    static bool Validate(const ShipyardWorkspaceLayoutState& state, std::string* error = nullptr);
};

} // namespace subspace
