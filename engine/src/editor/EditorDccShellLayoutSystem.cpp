#include "editor/EditorDccShellLayoutSystem.h"

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
    out.compact = height < static_cast<int>(860.0f * s);

    const float menuH = 24.0f * s;
    const float workspaceH = 30.0f * s;
    const float viewHeaderH = 26.0f * s;
    const float statusH = 24.0f * s;
    const float rightW = std::clamp(static_cast<float>(width) * 0.205f,
                                    300.0f * s, 390.0f * s);
    const float toolW = 40.0f * s;
    const float shelfH = (out.compact ? 154.0f : 188.0f) * s;

    out.applicationMenu = {0.0f, 0.0f, static_cast<float>(width), menuH};
    out.workspaceStrip = {0.0f, menuH, static_cast<float>(width), workspaceH};
    out.viewportHeader = {0.0f, menuH + workspaceH, static_cast<float>(width), viewHeaderH};
    out.statusBar = {0.0f, static_cast<float>(height) - statusH,
                     static_cast<float>(width), statusH};

    const float contentTop = out.viewportHeader.y + out.viewportHeader.height;
    const float contentBottom = out.statusBar.y;
    const float sidebarX = static_cast<float>(width) - rightW;
    const float shelfY = std::max(contentTop + 260.0f * s, contentBottom - shelfH);

    out.toolRail = {4.0f * s, contentTop + 4.0f * s, toolW, shelfY - contentTop - 8.0f * s};
    out.viewport = {out.toolRail.x + out.toolRail.width + 3.0f * s,
                    contentTop,
                    sidebarX - (out.toolRail.x + out.toolRail.width + 3.0f * s),
                    shelfY - contentTop};
    out.assetShelf = {out.viewport.x, shelfY,
                      sidebarX - out.viewport.x,
                      contentBottom - shelfY};

    const float outlinerH = std::clamp((contentBottom - contentTop) * 0.31f,
                                       180.0f * s, 260.0f * s);
    out.outliner = {sidebarX, contentTop, rightW, outlinerH};
    out.properties = {sidebarX, contentTop + outlinerH,
                      rightW, contentBottom - contentTop - outlinerH};
    return out;
}

} // namespace subspace
