#pragma once

#include "ship_editor/ShipyardAssetBrowserSystem.h"
#include "ship_editor/ShipyardWorkspaceSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ShipyardDccOutlinerMode {
    Hierarchy,
    FunctionalRole,
    ModuleClass
};

enum class ShipyardDccViewportShading {
    Solid,
    Material,
    Wireframe
};

struct ShipyardDccUiState {
    ShipyardAssetBrowserState assetBrowser = ShipyardAssetBrowserSystem::DefaultState();
    bool showAssetBrowser = true;
    bool showToolRail = true;
    bool showSidebar = true;
    bool showOutliner = true;
    bool showProperties = true;
    bool showStatusBar = true;
    bool maximizeViewport = false;
    bool showGrid = true;
    bool showGizmos = true;
    bool showSocketOverlay = false;
    bool showStatsOverlay = false;
    bool showShieldPreview = false;
    bool showHelpOverlay = false;
    bool commandPaletteOpen = false;
    bool pinProperties = false;
    ShipyardDccOutlinerMode outlinerMode = ShipyardDccOutlinerMode::Hierarchy;
    ShipyardDccViewportShading shading = ShipyardDccViewportShading::Material;
    std::size_t assetPreset = 0;
};

struct ShipyardDccOutlinerRow {
    std::size_t moduleIndex = 0;
    std::size_t depth = 0;
    std::string label;
    std::string group;
    bool selected = false;
    bool attached = false;
};

struct ShipyardDccCommandPaletteItem {
    std::string label;
    std::string shortcut;
    std::string group;
};

class ShipyardDccUiSystem {
public:
    static ShipyardDccUiState DefaultState();
    static void ResetLayout(ShipyardDccUiState& state);

    static const char* OutlinerModeName(ShipyardDccOutlinerMode mode);
    static const char* ShadingName(ShipyardDccViewportShading mode);
    static const char* AssetDensityName(ShipyardAssetBrowserDensity density);

    static ShipyardDccOutlinerMode NextOutlinerMode(ShipyardDccOutlinerMode mode);
    static ShipyardDccViewportShading NextShading(ShipyardDccViewportShading mode);
    static ShipyardAssetBrowserDensity NextAssetDensity(ShipyardAssetBrowserDensity density);

    static bool ApplyAssetPreset(ShipyardDccUiState& state, std::size_t preset);
    static std::size_t AssetPresetCount();
    static const char* AssetPresetName(std::size_t preset);

    static std::vector<ShipyardDccOutlinerRow> BuildOutlinerRows(
        const std::vector<ShipyardModuleRecord>& catalog,
        const ProceduralShipVisualRecipe& recipe,
        std::size_t selectedModule,
        ShipyardDccOutlinerMode mode);

    static std::vector<ShipyardDccCommandPaletteItem> CommandPalette();
    static std::vector<ShipyardWorkspaceMode> WorkspaceCycle();
};

} // namespace subspace
