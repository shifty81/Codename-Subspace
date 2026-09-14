#pragma once

#include "ship_editor/ShipyardActivityDockSystem.h"
#include "ship_editor/ShipyardAssetBrowserSystem.h"
#include "ship_editor/ShipyardCommandSystem.h"
#include "ship_editor/ShipyardPanelModelSystem.h"
#include "ship_editor/ShipyardUniversalSearchSystem.h"
#include "ship_editor/ShipyardWorkspaceLayoutSystem.h"

namespace subspace {

struct ShipyardProfessionalShellState {
    ShipyardWorkspaceLayoutState layout{};
    ShipyardActivityDockState activityDock{};
    ShipyardAssetBrowserState assetBrowser{};
    ShipyardPanelModel panelModel{};
    std::vector<ShipyardAssetBrowserItem> assetItems;
    std::vector<ShipyardSearchResult> searchResults;
    std::vector<ShipyardPanelDescriptor> visiblePanels;
    std::vector<ShipyardQuickActionDescriptor> quickActions;
    std::vector<ShipyardWorkspaceMode> primaryWorkspaces;
    std::vector<ShipyardWorkspaceMode> advancedWorkspaces;
};

/// Single render-facing projection for the normalized Shipyard shell.
/// Renderer code should consume this projection rather than recomputing panel,
/// asset, search, command, and layout state independently.
class ShipyardProfessionalShellSystem {
public:
    static ShipyardProfessionalShellState Create();
    static void Rebuild(ShipyardProfessionalShellState& shell,
                        const ShipyardDocument& document,
                        const ShipyardSession& session,
                        const ShipyardCommandSystem& commands,
                        const std::vector<ShipyardModuleRecord>& catalog,
                        const ShipyardModuleRecord* selectedParent,
                        const std::vector<EditorValidationMessage>& validation = {});
};

} // namespace subspace
