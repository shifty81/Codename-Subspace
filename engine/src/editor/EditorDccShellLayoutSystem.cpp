#include "editor/EditorDccShellLayoutSystem.h"
#include "editor/EditorForgeGuiStyleSystem.h"

#include <algorithm>

namespace subspace {

EditorDccShellLayout EditorDccShellLayoutSystem::Compute(int width, int height) {
    EditorDccShellLayout out;
    if (width < 1120 || height < 740) return out;

    out.valid = true;
    out.uiScale = std::clamp(std::min(static_cast<float>(width) / 1920.0f,
                                      static_cast<float>(height) / 1080.0f),
                             1.0f, 1.60f);
    const float s = out.uiScale;
    out.compact = height < static_cast<int>(900.0f * s);
    const auto metrics = EditorForgeGuiStyleSystem::Metrics(out.compact);

    // PASS1454-1465: one canonical metric authority derived from ForgeGUI_Core's
    // Creator Studio shell. No panel gets to invent its own header/row heights.
    const float menuH = metrics.applicationMenuHeight * s;
    const float workspaceH = metrics.workspaceTabHeight * s;
    const float viewHeaderH = metrics.viewportHeaderHeight * s;
    const float statusH = metrics.statusBarHeight * s;
    const float splitter = metrics.splitterWidth * s;
    const float rightW = std::clamp(static_cast<float>(width) * 0.205f,
                                    320.0f * s, 410.0f * s);
    const float toolW = metrics.toolRailWidth * s;
    const float shelfH = metrics.assetShelfHeight * s;

    out.applicationMenu = {0.0f, 0.0f, static_cast<float>(width), menuH};
    out.workspaceStrip = {0.0f, menuH, static_cast<float>(width), workspaceH};
    out.viewportHeader = {0.0f, menuH + workspaceH, static_cast<float>(width), viewHeaderH};
    out.statusBar = {0.0f, static_cast<float>(height) - statusH,
                     static_cast<float>(width), statusH};

    const float contentTop = out.viewportHeader.y + out.viewportHeader.height;
    const float contentBottom = out.statusBar.y;
    const float sidebarX = static_cast<float>(width) - rightW;
    const float minViewportHeight = 330.0f * s;
    const float shelfY = std::max(contentTop + minViewportHeight,
                                  contentBottom - shelfH);

    out.toolRail = {0.0f, contentTop, toolW, shelfY - contentTop};
    out.viewport = {toolW + splitter,
                    contentTop,
                    sidebarX - toolW - splitter,
                    shelfY - contentTop};
    out.assetShelf = {0.0f, shelfY,
                      sidebarX,
                      contentBottom - shelfY};

    // Outliner is intentionally shallower than the Properties surface. The
    // selected-object header and contextual property rows need vertical space
    // on 768/800 px screens without colliding with validation/status surfaces.
    const float availableRightH = contentBottom - contentTop;
    const float outlinerH = std::clamp(availableRightH * 0.285f,
                                       170.0f * s, 250.0f * s);
    out.outliner = {sidebarX, contentTop, rightW, outlinerH};
    out.properties = {sidebarX, contentTop + outlinerH + splitter,
                      rightW, availableRightH - outlinerH - splitter};
    return out;
}
} // namespace subspace
