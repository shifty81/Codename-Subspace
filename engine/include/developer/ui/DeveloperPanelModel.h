#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class DeveloperPanelKind {
    Console,
    Inspector,
    Assets,
    Diff,
    Diagnostics,
    Selection,
    Gizmos,
    Validation,
    AI,
    Unknown
};

struct DeveloperPanelDescriptor {
    std::string id;
    DeveloperPanelKind kind = DeveloperPanelKind::Unknown;
    std::string title;
    bool visible = false;
    bool docked = true;
    int order = 0;
};

class DeveloperPanelModel {
public:
    void RegisterPanel(DeveloperPanelDescriptor descriptor);
    bool TogglePanel(const std::string& id);
    bool SetVisible(const std::string& id, bool visible);
    bool IsVisible(const std::string& id) const;
    std::vector<DeveloperPanelDescriptor> GetPanels() const;
    std::vector<DeveloperPanelDescriptor> GetVisiblePanels() const;
    void RegisterDefaultPanels();
    void Clear();

private:
    std::unordered_map<std::string, DeveloperPanelDescriptor> _panels;
};

} // namespace subspace
