#include "editor/EditorForgeGuiStyleSystem.h"
#include <algorithm>

namespace subspace {

EditorForgeGuiMetrics EditorForgeGuiStyleSystem::Metrics(bool compact) {
    EditorForgeGuiMetrics m;
    // Shared row geometry feeds both paint and hit rectangles. Even compact
    // layouts retain readable targets; density must never mean clipped text.
    m.propertyRowHeight = 30.0f;
    m.assetRowHeight = 29.0f;
    m.toolRailWidth = 44.0f;
    m.iconSize = 17.0f;
    m.panelPadding = 7.0f;
    if (compact) {
        // ForgeGUI compact density is 0.90. Keep hit targets usable while
        // tightening chrome and rows enough for 768/800 px development screens.
        constexpr float d = 0.90f;
        m.applicationMenuHeight *= d;
        m.workspaceTabHeight *= d;
        m.viewportHeaderHeight *= d;
        m.panelHeaderHeight *= d;
        m.propertyRowHeight *= d;
        m.assetRowHeight *= d;
        m.statusBarHeight *= d;
        m.iconSize *= d;
        m.panelPadding *= d;
        m.outerGap *= d;
        m.toolRailWidth *= d;
        m.inspectorObjectHeaderHeight *= d;
        m.assetShelfHeight = 166.0f;
        m.propertyRowHeight = std::max(m.propertyRowHeight, 28.0f);
        m.assetRowHeight = std::max(m.assetRowHeight, 27.0f);
        m.toolRailWidth = std::max(m.toolRailWidth, 40.0f);
    }
    return m;
}

EditorForgeGuiPalette EditorForgeGuiStyleSystem::Palette() {
    // Values are normalized equivalents of ForgeGUI_Core's Forge Dark semantic
    // palette, expressed in Subspace float RGBA rather than Rust u8 RGBA.
    auto c=[](int r,int g,int b,int a=255){
        return SubspaceUiColor{r/255.0f,g/255.0f,b/255.0f,a/255.0f};
    };
    EditorForgeGuiPalette p;
    p.shell=c(13,18,26);
    p.menu=c(20,27,37);
    p.action=c(27,38,51);
    p.workspace=c(18,27,39);
    p.panel=c(23,33,45);
    p.panelRaised=c(34,47,63);
    p.panelRecessed=c(12,22,34);
    p.panelHeader=c(29,43,60);
    p.status=c(16,27,39);
    p.separator=c(53,72,92);
    p.text=c(231,238,247);
    p.textMuted=c(163,181,203);
    p.accent=c(90,170,244);
    p.success=c(73,218,145);
    p.warning=c(236,183,74);
    p.danger=c(240,95,109);
    return p;
}

} // namespace subspace
