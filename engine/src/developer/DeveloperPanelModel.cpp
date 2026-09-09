#include "developer/ui/DeveloperPanelModel.h"

#include <algorithm>

namespace subspace {

void DeveloperPanelModel::RegisterPanel(DeveloperPanelDescriptor descriptor) {
    _panels[descriptor.id] = std::move(descriptor);
}

bool DeveloperPanelModel::TogglePanel(const std::string& id) {
    auto it = _panels.find(id);
    if (it == _panels.end()) {
        return false;
    }
    it->second.visible = !it->second.visible;
    return true;
}

bool DeveloperPanelModel::SetVisible(const std::string& id, bool visible) {
    auto it = _panels.find(id);
    if (it == _panels.end()) {
        return false;
    }
    it->second.visible = visible;
    return true;
}

bool DeveloperPanelModel::IsVisible(const std::string& id) const {
    auto it = _panels.find(id);
    return it != _panels.end() && it->second.visible;
}

std::vector<DeveloperPanelDescriptor> DeveloperPanelModel::GetPanels() const {
    std::vector<DeveloperPanelDescriptor> result;
    result.reserve(_panels.size());
    for (const auto& entry : _panels) {
        result.push_back(entry.second);
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.order == b.order) {
            return a.id < b.id;
        }
        return a.order < b.order;
    });
    return result;
}

std::vector<DeveloperPanelDescriptor> DeveloperPanelModel::GetVisiblePanels() const {
    std::vector<DeveloperPanelDescriptor> result;
    for (const auto& panel : GetPanels()) {
        if (panel.visible) {
            result.push_back(panel);
        }
    }
    return result;
}

void DeveloperPanelModel::RegisterDefaultPanels() {
    RegisterPanel({"console", DeveloperPanelKind::Console, "Console", true, true, 10});
    RegisterPanel({"selection", DeveloperPanelKind::Selection, "Selection", true, true, 20});
    RegisterPanel({"inspector", DeveloperPanelKind::Inspector, "Inspector", true, true, 30});
    RegisterPanel({"assets", DeveloperPanelKind::Assets, "Assets / Hot Reload", false, true, 40});
    RegisterPanel({"diff", DeveloperPanelKind::Diff, "Runtime Diff", false, true, 50});
    RegisterPanel({"diagnostics", DeveloperPanelKind::Diagnostics, "Diagnostics", false, true, 60});
    RegisterPanel({"validation", DeveloperPanelKind::Validation, "Validation", false, true, 70});
    RegisterPanel({"ai", DeveloperPanelKind::AI, "AI Commands", false, true, 80});
}

void DeveloperPanelModel::Clear() {
    _panels.clear();
}

} // namespace subspace
