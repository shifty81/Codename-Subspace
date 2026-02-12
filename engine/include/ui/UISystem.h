#pragma once

#include "ui/UIPanel.h"
#include "ui/UIRenderer.h"
#include "core/ecs/SystemBase.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace subspace {

/// Top-level UI system — owns panels and orchestrates layout, input, and rendering.
///
/// Usage each frame:
///   1. uiSystem.HandleInput(mouseX, mouseY, clicked);
///   2. uiSystem.Update(deltaTime);
///   3. uiSystem.Render(renderer);
class UISystem : public SystemBase {
public:
    UISystem();

    /// ECS update (animates elements, etc.).
    void Update(float deltaTime) override;

    // --- Panel management ---

    /// Register a panel. Panels are rendered in insertion order.
    UIPanel* AddPanel(const std::string& id, std::shared_ptr<UIPanel> panel);

    /// Remove a panel by id. Returns true if found and removed.
    bool RemovePanel(const std::string& id);

    /// Get a panel by id. Returns nullptr if not found.
    UIPanel* GetPanel(const std::string& id) const;

    /// Get total number of registered panels.
    size_t GetPanelCount() const;

    /// Toggle a panel's visibility by id. Returns new visibility state.
    bool TogglePanel(const std::string& id);

    // --- Input ---

    /// Process mouse state for the current frame.
    void HandleInput(float mouseX, float mouseY, bool clicked);

    // --- Rendering ---

    /// Render all visible panels into the given renderer's command buffer.
    void Render(UIRenderer& renderer) const;

    // --- Screen size ---

    void SetScreenSize(float w, float h) { _screenWidth = w; _screenHeight = h; }
    float GetScreenWidth()  const { return _screenWidth; }
    float GetScreenHeight() const { return _screenHeight; }

private:
    /// Ordered list of panel ids (rendering order).
    std::vector<std::string> _panelOrder;
    /// Id → panel map.
    std::unordered_map<std::string, std::shared_ptr<UIPanel>> _panels;

    float _screenWidth  = 1920.0f;
    float _screenHeight = 1080.0f;
};

} // namespace subspace
