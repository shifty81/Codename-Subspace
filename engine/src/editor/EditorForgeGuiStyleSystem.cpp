#include "editor/EditorForgeGuiStyleSystem.h"

namespace subspace {

EditorForgeGuiMetrics EditorForgeGuiStyleSystem::Metrics(bool compact) {
    EditorForgeGuiMetrics m;
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
    p.shell=c(13,15,18);
    p.menu=c(23,26,31);
    p.action=c(27,31,36);
    p.workspace=c(18,21,25);
    p.panel=c(24,27,32);
    p.panelRaised=c(33,37,43);
    p.panelRecessed=c(11,13,16);
    p.panelHeader=c(31,35,41);
    p.status=c(15,17,20);
    p.separator=c(49,56,65);
    p.text=c(229,235,242);
    p.textMuted=c(148,160,174);
    p.accent=c(72,230,161);
    p.success=c(73,218,145);
    p.warning=c(236,183,74);
    p.danger=c(240,95,109);
    return p;
}

} // namespace subspace
