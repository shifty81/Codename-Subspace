#include "editor/EditorWorkspaceStateSystem.h"

#include <algorithm>

namespace subspace {
void EditorWorkspaceStateSystem::Set(EditorWorkspacePreferences p) {
    p.leftPanelWidth = std::clamp(p.leftPanelWidth, 180.0f, 640.0f);
    p.rightPanelWidth = std::clamp(p.rightPanelWidth, 220.0f, 720.0f);
    p.bottomPanelHeight = std::clamp(p.bottomPanelHeight, 100.0f, 420.0f);
    state_[static_cast<int>(p.workspace)] = std::move(p);
}

EditorWorkspacePreferences EditorWorkspaceStateSystem::Get(EditorWorkspaceKind w) const {
    const auto it = state_.find(static_cast<int>(w));
    if (it != state_.end()) return it->second;
    EditorWorkspacePreferences p;
    p.workspace = w;
    return p;
}

void EditorWorkspaceStateSystem::SetDockWorkspace(EditorDockWorkspace workspace) {
    std::string error;
    if (!EditorDockSystem::Validate(workspace, &error)) return;
    dockState_[static_cast<int>(workspace.workspace)] = std::move(workspace);
}

EditorDockWorkspace EditorWorkspaceStateSystem::GetDockWorkspace(EditorWorkspaceKind workspace) const {
    const auto it = dockState_.find(static_cast<int>(workspace));
    if (it != dockState_.end()) return it->second;
    return EditorDockSystem::CreateDefault(workspace);
}

EditorDockWorkspace& EditorWorkspaceStateSystem::MutableDockWorkspace(EditorWorkspaceKind workspace) {
    const int key = static_cast<int>(workspace);
    auto it = dockState_.find(key);
    if (it == dockState_.end()) it = dockState_.emplace(key, EditorDockSystem::CreateDefault(workspace)).first;
    return it->second;
}

void EditorWorkspaceStateSystem::Reset(EditorWorkspaceKind w) {
    state_.erase(static_cast<int>(w));
    dockState_.erase(static_cast<int>(w));
}
} // namespace subspace
