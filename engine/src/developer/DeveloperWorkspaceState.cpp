#include "developer/ui/DeveloperWorkspaceState.h"

#include <algorithm>

namespace subspace {

void DeveloperWorkspaceState::SetPanel(DeveloperPanelState panel)
{
    if (!panel.id.empty()) {
        _panels[panel.id] = std::move(panel);
    }
}

bool DeveloperWorkspaceState::HasPanel(const std::string& id) const
{
    return _panels.find(id) != _panels.end();
}

DeveloperPanelState* DeveloperWorkspaceState::FindPanel(const std::string& id)
{
    auto it = _panels.find(id);
    return it == _panels.end() ? nullptr : &it->second;
}

const DeveloperPanelState* DeveloperWorkspaceState::FindPanel(const std::string& id) const
{
    auto it = _panels.find(id);
    return it == _panels.end() ? nullptr : &it->second;
}

void DeveloperWorkspaceState::SetPanelVisible(const std::string& id, bool visible)
{
    auto* panel = FindPanel(id);
    if (panel) {
        panel->visible = visible;
    }
}

void DeveloperWorkspaceState::TogglePanel(const std::string& id)
{
    auto* panel = FindPanel(id);
    if (panel) {
        panel->visible = !panel->visible;
    }
}

std::vector<DeveloperPanelState> DeveloperWorkspaceState::GetPanels() const
{
    std::vector<DeveloperPanelState> result;
    result.reserve(_panels.size());
    for (const auto& kv : _panels) {
        result.push_back(kv.second);
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    return result;
}

void DeveloperWorkspaceState::Clear()
{
    _panels.clear();
}

} // namespace subspace
