#include "ui/UISystem.h"

namespace subspace {

UISystem::UISystem() : SystemBase("UISystem") {}

void UISystem::Update(float /*deltaTime*/) {
    // Re-layout all visible panels each frame so dynamic content stays aligned.
    for (const auto& id : _panelOrder) {
        auto it = _panels.find(id);
        if (it != _panels.end() && it->second->IsVisible()) {
            it->second->PerformLayout();
        }
    }
}

UIPanel* UISystem::AddPanel(const std::string& id, std::shared_ptr<UIPanel> panel) {
    if (!panel) return nullptr;

    // If the id already exists, replace it in-place.
    auto it = _panels.find(id);
    if (it != _panels.end()) {
        it->second = panel;
    } else {
        _panels[id] = panel;
        _panelOrder.push_back(id);
    }
    return panel.get();
}

bool UISystem::RemovePanel(const std::string& id) {
    auto it = _panels.find(id);
    if (it == _panels.end()) return false;

    _panels.erase(it);
    _panelOrder.erase(
        std::remove(_panelOrder.begin(), _panelOrder.end(), id),
        _panelOrder.end());
    return true;
}

UIPanel* UISystem::GetPanel(const std::string& id) const {
    auto it = _panels.find(id);
    if (it == _panels.end()) return nullptr;
    return it->second.get();
}

size_t UISystem::GetPanelCount() const {
    return _panels.size();
}

bool UISystem::TogglePanel(const std::string& id) {
    auto* panel = GetPanel(id);
    if (!panel) return false;
    panel->SetVisible(!panel->IsVisible());
    return panel->IsVisible();
}

void UISystem::HandleInput(float mouseX, float mouseY, bool clicked) {
    if (!clicked) return;

    // Process panels in reverse order (top-most rendered last → checked first).
    for (auto it = _panelOrder.rbegin(); it != _panelOrder.rend(); ++it) {
        auto pit = _panels.find(*it);
        if (pit == _panels.end()) continue;
        if (!pit->second->IsVisible()) continue;
        if (pit->second->HandleClick(mouseX, mouseY)) return;
    }
}

void UISystem::Render(UIRenderer& renderer) const {
    for (const auto& id : _panelOrder) {
        auto it = _panels.find(id);
        if (it == _panels.end()) continue;
        if (!it->second->IsVisible()) continue;

        std::vector<DrawCommand> commands;
        it->second->Render(commands);
        renderer.Submit(commands);
    }
}

} // namespace subspace
