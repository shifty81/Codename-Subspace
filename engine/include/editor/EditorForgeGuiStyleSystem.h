#pragma once

#include "ui/SubspaceUiFramework.h"

namespace subspace {

/// C++ projection of the useful ForgeGUI_Core creator-studio style contracts.
/// This is not a runtime dependency on the Rust/egui library: Subspace keeps its
/// native renderer while sharing the same semantic metrics and visual hierarchy.
struct EditorForgeGuiMetrics {
    float applicationMenuHeight = 27.0f;
    float workspaceTabHeight = 28.0f;
    float viewportHeaderHeight = 27.0f;
    float panelHeaderHeight = 27.0f;
    float propertyRowHeight = 27.0f;
    float assetRowHeight = 26.0f;
    float statusBarHeight = 27.0f;
    float splitterWidth = 3.0f;
    float iconSize = 15.0f;
    float panelPadding = 5.0f;
    float outerGap = 2.0f;
    float toolRailWidth = 38.0f;
    float inspectorObjectHeaderHeight = 58.0f;
    float assetShelfHeight = 190.0f;
};

struct EditorForgeGuiPalette {
    SubspaceUiColor shell{};
    SubspaceUiColor menu{};
    SubspaceUiColor action{};
    SubspaceUiColor workspace{};
    SubspaceUiColor panel{};
    SubspaceUiColor panelRaised{};
    SubspaceUiColor panelRecessed{};
    SubspaceUiColor panelHeader{};
    SubspaceUiColor status{};
    SubspaceUiColor separator{};
    SubspaceUiColor text{};
    SubspaceUiColor textMuted{};
    SubspaceUiColor accent{};
    SubspaceUiColor success{};
    SubspaceUiColor warning{};
    SubspaceUiColor danger{};
};

class EditorForgeGuiStyleSystem {
public:
    // Latest ForgeGUI_Core certified donor inspected for PASS1454-1465.
    static constexpr const char* DonorCommit() { return "532f7e1"; }
    static constexpr const char* DonorProfile() { return "ForgeGUI_Core 0.4.8 / Creator Studio"; }

    static EditorForgeGuiMetrics Metrics(bool compact = false);
    static EditorForgeGuiPalette Palette();
};

} // namespace subspace
