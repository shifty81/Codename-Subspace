#pragma once

#include "ui/UIElement.h"
#include "ui/UITypes.h"

#include <string>
#include <vector>

namespace subspace {

/// Layout direction for child elements within a panel.
enum class LayoutDirection {
    Vertical,
    Horizontal
};

/// A container that holds child UI elements and manages their layout.
class UIPanel : public UIElement {
public:
    UIPanel();

    /// Add a child element. Returns a raw pointer for convenience.
    UIElement* AddChild(UIElementPtr child);

    /// Remove a child by id. Returns true if found and removed.
    bool RemoveChild(const std::string& id);

    /// Find a child by id. Returns nullptr if not found.
    UIElement* FindChild(const std::string& id) const;

    /// Get all children.
    const std::vector<UIElementPtr>& GetChildren() const { return _children; }

    /// Clear all children.
    void ClearChildren();

    /// Get number of children.
    size_t GetChildCount() const { return _children.size(); }

    // --- Styling ---

    const Color& GetBackgroundColor() const { return _bgColor; }
    void SetBackgroundColor(const Color& c) { _bgColor = c; }

    const Color& GetBorderColor() const { return _borderColor; }
    void SetBorderColor(const Color& c) { _borderColor = c; }

    bool HasBorder() const { return _hasBorder; }
    void SetHasBorder(bool b) { _hasBorder = b; }

    const std::string& GetTitle() const { return _title; }
    void SetTitle(const std::string& t) { _title = t; }

    // --- Layout ---

    LayoutDirection GetLayoutDirection() const { return _layoutDir; }
    void SetLayoutDirection(LayoutDirection d) { _layoutDir = d; }

    float GetPadding() const { return _padding; }
    void  SetPadding(float p) { _padding = p; }

    float GetSpacing() const { return _spacing; }
    void  SetSpacing(float s) { _spacing = s; }

    /// Recompute child positions based on layout direction, padding, and spacing.
    void PerformLayout();

    // --- Rendering / Input ---

    void Render(std::vector<DrawCommand>& commands) const override;
    bool HandleClick(float x, float y) override;

private:
    std::vector<UIElementPtr> _children;

    Color           _bgColor     = Color(0.1f, 0.1f, 0.15f, 0.85f);
    Color           _borderColor = Color(0.4f, 0.4f, 0.5f, 1.0f);
    bool            _hasBorder   = true;
    std::string     _title;

    LayoutDirection _layoutDir = LayoutDirection::Vertical;
    float           _padding   = 8.0f;
    float           _spacing   = 4.0f;
};

} // namespace subspace
