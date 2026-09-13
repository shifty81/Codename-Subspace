#include "editor/SubspaceEditorLayoutSystem.h"

#include <algorithm>

namespace subspace {
namespace {
EditorPanelSlot SlotFor(const std::string& id) {
    if (id == "asset_browser") return EditorPanelSlot::AssetBrowser;
    if (id == "viewport") return EditorPanelSlot::Viewport;
    if (id == "inspector") return EditorPanelSlot::Inspector;
    if (id == "outliner") return EditorPanelSlot::Outliner;
    if (id == "validation") return EditorPanelSlot::Validation;
    if (id == "console") return EditorPanelSlot::Console;
    if (id == "pcg") return EditorPanelSlot::Pcg;
    if (id == "jobs") return EditorPanelSlot::Jobs;
    if (id == "history") return EditorPanelSlot::History;
    if (id == "selection") return EditorPanelSlot::Selection;
    if (id == "properties") return EditorPanelSlot::Properties;
    if (id == "diagnostics") return EditorPanelSlot::Diagnostics;
    if (id == "runtime_diff") return EditorPanelSlot::RuntimeDiff;
    if (id == "ai") return EditorPanelSlot::AI;
    return EditorPanelSlot::Custom;
}
}

EditorWorkspaceLayout SubspaceEditorLayoutSystem::Build(int w, int h, bool showOutliner, bool showBottom) {
    auto dock = EditorDockSystem::CreateDefault(EditorWorkspaceKind::Shipyard);
    if (!showOutliner) EditorDockSystem::ClosePanel(dock, "outliner");
    if (showBottom) {
        if (auto* bottom = EditorDockSystem::FindNode(dock, "bottom")) bottom->collapsed = false;
        EditorDockSystem::OpenPanel(dock, "validation");
        EditorDockSystem::ActivatePanel(dock, "validation");
    }
    return BuildDocked(dock, w, h);
}

EditorWorkspaceLayout SubspaceEditorLayoutSystem::BuildDocked(const EditorDockWorkspace& dock, int w, int h, float topBarHeight) {
    EditorWorkspaceLayout out;
    out.viewportWidth = std::max(640, w);
    out.viewportHeight = std::max(420, h);
    out.topBarHeight = std::clamp(topBarHeight, 24.0f, 96.0f);

    const auto materialized = EditorDockSystem::Materialize(dock, out.viewportWidth, out.viewportHeight, out.topBarHeight);
    for (const auto& panel : materialized) {
        const auto* descriptor = EditorDockSystem::FindPanel(dock, panel.panelId);
        EditorPanelLayout p;
        p.slot = SlotFor(panel.panelId);
        p.id = panel.panelId;
        p.x = panel.rect.x;
        p.y = panel.rect.y;
        p.width = panel.rect.width;
        p.height = panel.rect.height;
        p.visible = panel.visible;
        p.collapsible = descriptor ? descriptor->canClose : true;
        p.active = panel.active;
        p.floating = panel.floating;
        p.dockLeafId = panel.leafId;
        out.panels.push_back(std::move(p));
    }

    auto widthOf = [&](const std::string& leaf) {
        for (const auto& p : out.panels) if (p.dockLeafId == leaf && p.visible) return p.width;
        return 0.0f;
    };
    auto heightOf = [&](const std::string& leaf) {
        for (const auto& p : out.panels) if (p.dockLeafId == leaf && p.visible) return p.height;
        return 0.0f;
    };
    out.leftWidth = widthOf("left");
    out.rightWidth = widthOf("right");
    out.bottomHeight = heightOf("bottom");
    return out;
}

bool SubspaceEditorLayoutSystem::Validate(const EditorWorkspaceLayout& o, std::string* error) {
    if (o.viewportWidth < 640 || o.viewportHeight < 420) {
        if (error) *error = "editor viewport below minimum supported dimensions";
        return false;
    }
    for (const auto& p : o.panels) {
        if (!p.visible) continue;
        if (p.width <= 0 || p.height <= 0 || p.x < 0 || p.y < 0 || p.x + p.width > o.viewportWidth + .01f || p.y + p.height > o.viewportHeight + .01f) {
            if (error) *error = "editor panel lies outside workspace bounds: " + p.id;
            return false;
        }
    }
    return true;
}
} // namespace subspace
