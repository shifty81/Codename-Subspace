#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct DeveloperPanelState {
    std::string id;
    std::string title;
    bool visible = true;
    bool collapsed = false;
    float x = 0.0f;
    float y = 0.0f;
    float width = 360.0f;
    float height = 240.0f;
};

class DeveloperWorkspaceState {
public:
    void SetPanel(DeveloperPanelState panel);
    bool HasPanel(const std::string& id) const;
    DeveloperPanelState* FindPanel(const std::string& id);
    const DeveloperPanelState* FindPanel(const std::string& id) const;
    void SetPanelVisible(const std::string& id, bool visible);
    void TogglePanel(const std::string& id);
    std::vector<DeveloperPanelState> GetPanels() const;
    void Clear();

private:
    std::unordered_map<std::string, DeveloperPanelState> _panels;
};

} // namespace subspace
