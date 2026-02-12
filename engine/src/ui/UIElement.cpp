#include "ui/UIElement.h"

namespace subspace {

// --------------------------------------------------------------------------
// UILabel
// --------------------------------------------------------------------------

void UILabel::Render(std::vector<DrawCommand>& commands) const {
    if (!_visible || _text.empty()) return;

    DrawCommand cmd;
    cmd.type     = DrawCommandType::Text;
    cmd.rect     = _bounds;
    cmd.color    = _color;
    cmd.text     = _text;
    cmd.fontSize = _fontSize;
    cmd.p1       = {_bounds.x, _bounds.y};
    commands.push_back(cmd);
}

// --------------------------------------------------------------------------
// UIButton
// --------------------------------------------------------------------------

void UIButton::Render(std::vector<DrawCommand>& commands) const {
    if (!_visible) return;

    // Background
    DrawCommand bg;
    bg.type  = DrawCommandType::FilledRect;
    bg.rect  = _bounds;
    bg.color = _bgColor;
    commands.push_back(bg);

    // Border
    DrawCommand border;
    border.type      = DrawCommandType::OutlineRect;
    border.rect      = _bounds;
    border.color     = Color(0.6f, 0.6f, 0.7f, 1.0f);
    border.lineWidth = 1.0f;
    commands.push_back(border);

    // Label text (centered horizontally in the button)
    if (!_label.empty()) {
        DrawCommand text;
        text.type     = DrawCommandType::Text;
        text.rect     = _bounds;
        text.color    = _textColor;
        text.text     = _label;
        text.fontSize = 14;
        text.p1       = {_bounds.x + 4.0f, _bounds.y + 2.0f};
        commands.push_back(text);
    }
}

bool UIButton::HandleClick(float x, float y) {
    if (!_visible || !_enabled) return false;
    if (!_bounds.Contains(x, y)) return false;
    if (_onClick) _onClick();
    return true;
}

// --------------------------------------------------------------------------
// UIProgressBar
// --------------------------------------------------------------------------

void UIProgressBar::Render(std::vector<DrawCommand>& commands) const {
    if (!_visible) return;

    // Background
    DrawCommand bg;
    bg.type  = DrawCommandType::FilledRect;
    bg.rect  = _bounds;
    bg.color = _bgColor;
    commands.push_back(bg);

    // Fill
    Color fillColor = _fillColor;
    if (_autoColor) {
        if (_value > 0.6f)      fillColor = Color::Green();
        else if (_value > 0.3f) fillColor = Color::Yellow();
        else                    fillColor = Color::Red();
    }

    if (_value > 0.0f) {
        DrawCommand fill;
        fill.type  = DrawCommandType::FilledRect;
        fill.rect  = {_bounds.x, _bounds.y, _bounds.width * _value, _bounds.height};
        fill.color = fillColor;
        commands.push_back(fill);
    }

    // Border
    DrawCommand border;
    border.type      = DrawCommandType::OutlineRect;
    border.rect      = _bounds;
    border.color     = Color(0.5f, 0.5f, 0.6f, 1.0f);
    border.lineWidth = 1.0f;
    commands.push_back(border);

    // Label
    if (!_label.empty()) {
        DrawCommand text;
        text.type     = DrawCommandType::Text;
        text.rect     = _bounds;
        text.color    = Color::White();
        text.text     = _label;
        text.fontSize = 12;
        text.p1       = {_bounds.x + 4.0f, _bounds.y + 1.0f};
        commands.push_back(text);
    }
}

// --------------------------------------------------------------------------
// UISeparator
// --------------------------------------------------------------------------

void UISeparator::Render(std::vector<DrawCommand>& commands) const {
    if (!_visible) return;

    DrawCommand cmd;
    cmd.type      = DrawCommandType::Line;
    cmd.color     = _color;
    cmd.p1        = {_bounds.x, _bounds.y + _bounds.height * 0.5f};
    cmd.p2        = {_bounds.x + _bounds.width, _bounds.y + _bounds.height * 0.5f};
    cmd.lineWidth = 1.0f;
    commands.push_back(cmd);
}

// --------------------------------------------------------------------------
// UICheckbox
// --------------------------------------------------------------------------

void UICheckbox::Render(std::vector<DrawCommand>& commands) const {
    if (!_visible) return;

    float boxSize = _bounds.height;
    Rect boxRect = {_bounds.x, _bounds.y, boxSize, boxSize};

    // Box background
    DrawCommand bg;
    bg.type  = DrawCommandType::FilledRect;
    bg.rect  = boxRect;
    bg.color = Color(0.15f, 0.15f, 0.15f, 0.9f);
    commands.push_back(bg);

    // Box border
    DrawCommand border;
    border.type      = DrawCommandType::OutlineRect;
    border.rect      = boxRect;
    border.color     = Color(0.6f, 0.6f, 0.7f, 1.0f);
    border.lineWidth = 1.0f;
    commands.push_back(border);

    // Check mark (filled inner rect)
    if (_checked) {
        float inset = boxSize * 0.25f;
        DrawCommand check;
        check.type  = DrawCommandType::FilledRect;
        check.rect  = {boxRect.x + inset, boxRect.y + inset,
                       boxSize - inset * 2, boxSize - inset * 2};
        check.color = Color::Cyan();
        commands.push_back(check);
    }

    // Label text
    if (!_label.empty()) {
        DrawCommand text;
        text.type     = DrawCommandType::Text;
        text.color    = Color::White();
        text.text     = _label;
        text.fontSize = 14;
        text.p1       = {_bounds.x + boxSize + 6.0f, _bounds.y};
        commands.push_back(text);
    }
}

bool UICheckbox::HandleClick(float x, float y) {
    if (!_visible || !_enabled) return false;
    if (!_bounds.Contains(x, y)) return false;
    _checked = !_checked;
    if (_onChange) _onChange(_checked);
    return true;
}

} // namespace subspace
