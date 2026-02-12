#include "ui/UIPanel.h"

#include <algorithm>

namespace subspace {

UIPanel::UIPanel() : UIElement(UIElementType::Panel) {}

UIElement* UIPanel::AddChild(UIElementPtr child) {
    if (!child) return nullptr;
    _children.push_back(child);
    return child.get();
}

bool UIPanel::RemoveChild(const std::string& id) {
    auto it = std::find_if(_children.begin(), _children.end(),
                           [&id](const UIElementPtr& e) { return e->GetId() == id; });
    if (it == _children.end()) return false;
    _children.erase(it);
    return true;
}

UIElement* UIPanel::FindChild(const std::string& id) const {
    for (const auto& child : _children) {
        if (child->GetId() == id) return child.get();
    }
    return nullptr;
}

void UIPanel::ClearChildren() {
    _children.clear();
}

void UIPanel::PerformLayout() {
    float titleBarHeight = _title.empty() ? 0.0f : 22.0f;
    float cursor = _padding + titleBarHeight;
    float contentWidth = _bounds.width - _padding * 2;

    for (auto& child : _children) {
        if (!child->IsVisible()) continue;

        Rect childBounds = child->GetBounds();

        if (_layoutDir == LayoutDirection::Vertical) {
            childBounds.x = _bounds.x + _padding;
            childBounds.y = _bounds.y + cursor;
            if (childBounds.width <= 0) childBounds.width = contentWidth;
            child->SetBounds(childBounds);
            cursor += childBounds.height + _spacing;
        } else {
            childBounds.x = _bounds.x + cursor;
            childBounds.y = _bounds.y + _padding + titleBarHeight;
            child->SetBounds(childBounds);
            cursor += childBounds.width + _spacing;
        }
    }
}

void UIPanel::Render(std::vector<DrawCommand>& commands) const {
    if (!_visible) return;

    // Background
    DrawCommand bg;
    bg.type  = DrawCommandType::FilledRect;
    bg.rect  = _bounds;
    bg.color = _bgColor;
    commands.push_back(bg);

    // Border
    if (_hasBorder) {
        DrawCommand border;
        border.type      = DrawCommandType::OutlineRect;
        border.rect      = _bounds;
        border.color     = _borderColor;
        border.lineWidth = 1.0f;
        commands.push_back(border);
    }

    // Title bar
    if (!_title.empty()) {
        float titleBarHeight = 22.0f;
        DrawCommand titleBg;
        titleBg.type  = DrawCommandType::FilledRect;
        titleBg.rect  = {_bounds.x, _bounds.y, _bounds.width, titleBarHeight};
        titleBg.color = Color(0.2f, 0.2f, 0.3f, 0.95f);
        commands.push_back(titleBg);

        DrawCommand titleText;
        titleText.type     = DrawCommandType::Text;
        titleText.color    = Color::White();
        titleText.text     = _title;
        titleText.fontSize = 14;
        titleText.p1       = {_bounds.x + 6.0f, _bounds.y + 3.0f};
        commands.push_back(titleText);
    }

    // Render children
    for (const auto& child : _children) {
        child->Render(commands);
    }
}

bool UIPanel::HandleClick(float x, float y) {
    if (!_visible || !_enabled) return false;
    if (!_bounds.Contains(x, y)) return false;

    // Propagate to children in reverse order (top-most first)
    for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
        if ((*it)->HandleClick(x, y)) return true;
    }
    return true;  // Panel itself consumes the click to prevent pass-through
}

} // namespace subspace
