#include "editor/EditorDockSystem.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace subspace {
namespace {

bool Contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

void RemoveTab(EditorDockWorkspace& workspace, const std::string& panelId) {
    for (auto& node : workspace.nodes) {
        if (node.split) continue;
        const auto oldSize = node.tabs.size();
        node.tabs.erase(std::remove(node.tabs.begin(), node.tabs.end(), panelId), node.tabs.end());
        if (node.tabs.size() != oldSize && node.activeTabId == panelId) {
            node.activeTabId = node.tabs.empty() ? std::string{} : node.tabs.front();
        }
    }
}

void RemoveFloating(EditorDockWorkspace& workspace, const std::string& panelId) {
    workspace.floatingPanels.erase(
        std::remove_if(workspace.floatingPanels.begin(), workspace.floatingPanels.end(), [&](const auto& p) { return p.panelId == panelId; }),
        workspace.floatingPanels.end());
}

const EditorDockNode* FindLeafContaining(const EditorDockWorkspace& workspace, const std::string& panelId) {
    for (const auto& node : workspace.nodes) {
        if (!node.split && Contains(node.tabs, panelId)) return &node;
    }
    return nullptr;
}

void LayoutNode(const EditorDockWorkspace& workspace,
                const std::string& nodeId,
                const EditorDockRect& rect,
                std::vector<EditorDockPanelLayout>& out) {
    const auto* node = EditorDockSystem::FindNode(workspace, nodeId);
    if (!node || node->collapsed) return;

    if (node->split) {
        const float ratio = std::clamp(node->ratio, 0.08f, 0.92f);
        EditorDockRect first = rect;
        EditorDockRect second = rect;
        if (node->axis == EditorDockSplitAxis::Horizontal) {
            first.width = rect.width * ratio;
            second.x = rect.x + first.width;
            second.width = std::max(0.0f, rect.width - first.width);
        } else {
            first.height = rect.height * ratio;
            second.y = rect.y + first.height;
            second.height = std::max(0.0f, rect.height - first.height);
        }
        LayoutNode(workspace, node->firstChildId, first, out);
        LayoutNode(workspace, node->secondChildId, second, out);
        return;
    }

    for (const auto& panelId : node->tabs) {
        const auto* panel = EditorDockSystem::FindPanel(workspace, panelId);
        if (!panel || !panel->visible) continue;
        out.push_back({panelId, node->id, rect, true, node->activeTabId == panelId, false});
    }
}

void RegisterDefaults(EditorDockWorkspace& w) {
    auto panel = [&](const char* id, const char* title, const char* leaf, bool visible, bool canClose = true, bool canFloat = true) {
        w.panels.push_back({id, title, leaf, visible, canClose, canFloat, 150.0f, 90.0f});
    };
    panel("asset_browser", "Assets", "left", true);
    panel("outliner", "Outliner", "left", true);
    panel("viewport", "Viewport", "center", true, false, false);
    panel("inspector", "Inspector", "right", true);
    panel("selection", "Selection", "right", false);
    panel("properties", "Properties", "right", false);
    panel("console", "Console", "bottom", false);
    panel("validation", "Validation", "bottom", false);
    panel("diagnostics", "Diagnostics", "bottom", false);
    panel("jobs", "Jobs", "bottom", false);
    panel("history", "History", "bottom", false);
    panel("pcg", "PCG", "bottom", false);
    panel("runtime_diff", "Runtime Diff", "bottom", false);
    panel("ai", "AI Commands", "bottom", false);
}

} // namespace

EditorDockWorkspace EditorDockSystem::CreateDefault(EditorWorkspaceKind workspace) {
    EditorDockWorkspace w;
    w.workspace = workspace;
    w.rootNodeId = "root";
    RegisterDefaults(w);

    w.nodes.push_back({"root", true, EditorDockSplitAxis::Vertical, 0.78f, "main", "bottom", {}, {}, false});
    w.nodes.push_back({"main", true, EditorDockSplitAxis::Horizontal, 0.20f, "left", "center_right", {}, {}, false});
    w.nodes.push_back({"center_right", true, EditorDockSplitAxis::Horizontal, 0.72f, "center", "right", {}, {}, false});
    w.nodes.push_back({"left", false, EditorDockSplitAxis::Horizontal, 0.5f, {}, {}, {"asset_browser", "outliner"}, "asset_browser", false});
    w.nodes.push_back({"center", false, EditorDockSplitAxis::Horizontal, 0.5f, {}, {}, {"viewport"}, "viewport", false});
    w.nodes.push_back({"right", false, EditorDockSplitAxis::Horizontal, 0.5f, {}, {}, {"inspector", "selection", "properties"}, "inspector", false});
    w.nodes.push_back({"bottom", false, EditorDockSplitAxis::Horizontal, 0.5f, {}, {}, {"console", "validation", "diagnostics", "jobs", "history", "pcg", "runtime_diff", "ai"}, "console", true});
    return w;
}

EditorDockPanelDescriptor* EditorDockSystem::FindPanel(EditorDockWorkspace& w, const std::string& id) {
    for (auto& p : w.panels) if (p.id == id) return &p;
    return nullptr;
}
const EditorDockPanelDescriptor* EditorDockSystem::FindPanel(const EditorDockWorkspace& w, const std::string& id) {
    for (const auto& p : w.panels) if (p.id == id) return &p;
    return nullptr;
}
EditorDockNode* EditorDockSystem::FindNode(EditorDockWorkspace& w, const std::string& id) {
    for (auto& n : w.nodes) if (n.id == id) return &n;
    return nullptr;
}
const EditorDockNode* EditorDockSystem::FindNode(const EditorDockWorkspace& w, const std::string& id) {
    for (const auto& n : w.nodes) if (n.id == id) return &n;
    return nullptr;
}

bool EditorDockSystem::RegisterPanel(EditorDockWorkspace& w, EditorDockPanelDescriptor panel) {
    if (panel.id.empty() || FindPanel(w, panel.id)) return false;
    if (panel.defaultLeafId.empty()) panel.defaultLeafId = "bottom";
    w.panels.push_back(std::move(panel));
    return true;
}

bool EditorDockSystem::OpenPanel(EditorDockWorkspace& w, const std::string& panelId) {
    auto* panel = FindPanel(w, panelId);
    if (!panel) return false;
    panel->visible = true;
    if (IsFloating(w, panelId)) return true;
    auto leafId = LeafForPanel(w, panelId);
    if (leafId.empty()) leafId = panel->defaultLeafId;
    auto* leaf = FindNode(w, leafId);
    if (!leaf || leaf->split) return false;
    if (!Contains(leaf->tabs, panelId)) leaf->tabs.push_back(panelId);
    leaf->collapsed = false;
    leaf->activeTabId = panelId;
    return true;
}

bool EditorDockSystem::ClosePanel(EditorDockWorkspace& w, const std::string& panelId) {
    auto* panel = FindPanel(w, panelId);
    if (!panel || !panel->canClose) return false;
    panel->visible = false;
    RemoveFloating(w, panelId);
    for (auto& node : w.nodes) {
        if (!node.split && node.activeTabId == panelId) {
            node.activeTabId.clear();
            for (const auto& candidate : node.tabs) {
                const auto* p = FindPanel(w, candidate);
                if (p && p->visible) { node.activeTabId = candidate; break; }
            }
        }
    }
    return true;
}

bool EditorDockSystem::TogglePanel(EditorDockWorkspace& w, const std::string& panelId) {
    const auto* p = FindPanel(w, panelId);
    if (!p) return false;
    return p->visible ? ClosePanel(w, panelId) : OpenPanel(w, panelId);
}

bool EditorDockSystem::ActivatePanel(EditorDockWorkspace& w, const std::string& panelId) {
    auto* panel = FindPanel(w, panelId);
    if (!panel) return false;
    if (!panel->visible && !OpenPanel(w, panelId)) return false;
    if (IsFloating(w, panelId)) return true;
    auto* leaf = FindNode(w, LeafForPanel(w, panelId));
    if (!leaf || leaf->split) return false;
    leaf->activeTabId = panelId;
    leaf->collapsed = false;
    return true;
}

bool EditorDockSystem::MovePanel(EditorDockWorkspace& w, const std::string& panelId, const std::string& targetLeafId, bool activate) {
    auto* panel = FindPanel(w, panelId);
    auto* leaf = FindNode(w, targetLeafId);
    if (!panel || !leaf || leaf->split) return false;
    RemoveFloating(w, panelId);
    RemoveTab(w, panelId);
    leaf = FindNode(w, targetLeafId);
    if (!leaf) return false;
    leaf->tabs.push_back(panelId);
    leaf->collapsed = false;
    panel->visible = true;
    if (activate) leaf->activeTabId = panelId;
    return true;
}

bool EditorDockSystem::SplitLeaf(EditorDockWorkspace& w,
                                 const std::string& leafId,
                                 const std::string& newLeafId,
                                 EditorDockSplitAxis axis,
                                 float ratio,
                                 const std::string& panelId,
                                 bool placeNewAfter) {
    if (newLeafId.empty() || FindNode(w, newLeafId)) return false;
    auto* leaf = FindNode(w, leafId);
    auto* panel = FindPanel(w, panelId);
    if (!leaf || leaf->split || !panel) return false;

    const std::string oldChildId = leafId + ".content";
    if (FindNode(w, oldChildId)) return false;
    EditorDockNode oldContent = *leaf;
    oldContent.id = oldChildId;
    EditorDockNode newContent;
    newContent.id = newLeafId;
    newContent.tabs = {panelId};
    newContent.activeTabId = panelId;

    RemoveFloating(w, panelId);
    RemoveTab(w, panelId);
    oldContent = *FindNode(w, leafId);
    oldContent.id = oldChildId;

    *leaf = {};
    leaf->id = leafId;
    leaf->split = true;
    leaf->axis = axis;
    leaf->ratio = std::clamp(ratio, 0.08f, 0.92f);
    leaf->firstChildId = placeNewAfter ? oldChildId : newLeafId;
    leaf->secondChildId = placeNewAfter ? newLeafId : oldChildId;
    w.nodes.push_back(std::move(oldContent));
    w.nodes.push_back(std::move(newContent));
    panel = FindPanel(w, panelId);
    if (panel) panel->visible = true;
    return true;
}

bool EditorDockSystem::FloatPanel(EditorDockWorkspace& w, const std::string& panelId, EditorDockRect rect) {
    auto* panel = FindPanel(w, panelId);
    if (!panel || !panel->canFloat || rect.width <= 0.0f || rect.height <= 0.0f) return false;
    RemoveTab(w, panelId);
    RemoveFloating(w, panelId);
    panel->visible = true;
    rect.width = std::max(rect.width, panel->minWidth);
    rect.height = std::max(rect.height, panel->minHeight);
    w.floatingPanels.push_back({panelId, rect});
    return true;
}

bool EditorDockSystem::DockPanel(EditorDockWorkspace& w, const std::string& panelId, const std::string& targetLeafId, bool activate) {
    return MovePanel(w, panelId, targetLeafId, activate);
}

std::string EditorDockSystem::LeafForPanel(const EditorDockWorkspace& w, const std::string& panelId) {
    const auto* leaf = FindLeafContaining(w, panelId);
    return leaf ? leaf->id : std::string{};
}

bool EditorDockSystem::IsFloating(const EditorDockWorkspace& w, const std::string& panelId) {
    return std::any_of(w.floatingPanels.begin(), w.floatingPanels.end(), [&](const auto& p) { return p.panelId == panelId; });
}

std::vector<std::string> EditorDockSystem::OpenPanelIds(const EditorDockWorkspace& w) {
    std::vector<std::string> out;
    for (const auto& p : w.panels) if (p.visible) out.push_back(p.id);
    std::sort(out.begin(), out.end());
    return out;
}

std::vector<EditorDockPanelLayout> EditorDockSystem::Materialize(const EditorDockWorkspace& w, int width, int height, float topInset) {
    std::vector<EditorDockPanelLayout> out;
    if (width <= 0 || height <= 0 || topInset < 0.0f || topInset >= static_cast<float>(height)) return out;
    LayoutNode(w, w.rootNodeId, {0.0f, topInset, static_cast<float>(width), static_cast<float>(height) - topInset}, out);
    for (const auto& floating : w.floatingPanels) {
        const auto* panel = FindPanel(w, floating.panelId);
        if (!panel || !panel->visible) continue;
        EditorDockRect r = floating.rect;
        r.x = std::clamp(r.x, 0.0f, std::max(0.0f, static_cast<float>(width) - panel->minWidth));
        r.y = std::clamp(r.y, topInset, std::max(topInset, static_cast<float>(height) - panel->minHeight));
        r.width = std::clamp(r.width, panel->minWidth, static_cast<float>(width));
        r.height = std::clamp(r.height, panel->minHeight, static_cast<float>(height) - topInset);
        out.push_back({panel->id, {}, r, true, true, true});
    }
    return out;
}

bool EditorDockSystem::Validate(const EditorDockWorkspace& w, std::string* error) {
    if (w.rootNodeId.empty() || !FindNode(w, w.rootNodeId)) {
        if (error) *error = "dock workspace has no valid root node";
        return false;
    }
    std::unordered_set<std::string> panelIds;
    for (const auto& p : w.panels) {
        if (p.id.empty() || !panelIds.insert(p.id).second) {
            if (error) *error = "dock workspace contains an empty or duplicate panel id";
            return false;
        }
    }
    std::unordered_set<std::string> nodeIds;
    std::unordered_set<std::string> ownedPanels;
    for (const auto& n : w.nodes) {
        if (n.id.empty() || !nodeIds.insert(n.id).second) {
            if (error) *error = "dock workspace contains an empty or duplicate node id";
            return false;
        }
        if (n.split) {
            if (n.firstChildId.empty() || n.secondChildId.empty() || n.firstChildId == n.secondChildId || n.ratio <= 0.0f || n.ratio >= 1.0f) {
                if (error) *error = "dock split node is invalid: " + n.id;
                return false;
            }
        } else {
            for (const auto& panelId : n.tabs) {
                if (!FindPanel(w, panelId)) {
                    if (error) *error = "dock leaf references unknown panel: " + panelId;
                    return false;
                }
                if (!ownedPanels.insert(panelId).second) {
                    if (error) *error = "dock panel appears in more than one leaf/floating host: " + panelId;
                    return false;
                }
            }
            if (!n.activeTabId.empty() && !Contains(n.tabs, n.activeTabId)) {
                if (error) *error = "dock leaf active tab is not owned by leaf: " + n.id;
                return false;
            }
        }
    }
    for (const auto& n : w.nodes) {
        if (!n.split) continue;
        if (!FindNode(w, n.firstChildId) || !FindNode(w, n.secondChildId)) {
            if (error) *error = "dock split references a missing child: " + n.id;
            return false;
        }
    }
    for (const auto& f : w.floatingPanels) {
        const auto* p = FindPanel(w, f.panelId);
        if (!p || !p->canFloat || f.rect.width <= 0.0f || f.rect.height <= 0.0f || !ownedPanels.insert(f.panelId).second) {
            if (error) *error = "floating dock panel is invalid or duplicated: " + f.panelId;
            return false;
        }
    }
    return true;
}

} // namespace subspace
