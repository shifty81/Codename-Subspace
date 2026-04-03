#include "ui/widgets/Widgets.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ButtonWidget
// ---------------------------------------------------------------------------

ButtonWidget::ButtonWidget(const std::string& id,
                           const std::string& label)
    : Widget(id), _label(label) {}

void ButtonWidget::Render(std::vector<DrawCommand>& cmds) const {
    if (!_visible) return;

    DrawCommand bg;
    bg.type  = DrawCommandType::FilledRect;
    bg.rect  = _bounds;
    bg.color = _bgColor;
    cmds.push_back(bg);

    if (!_label.empty()) {
        DrawCommand txt;
        txt.type  = DrawCommandType::Text;
        txt.rect  = Rect{_bounds.x + 4.0f, _bounds.y + 4.0f, 0.0f, 0.0f};
        txt.text  = _label;
        txt.color = _textColor;
        cmds.push_back(txt);
    }

    Widget::Render(cmds); // children
}

bool ButtonWidget::HandleClick(float x, float y) {
    if (!_visible || !_enabled) return false;
    if (x >= _bounds.x && x <= _bounds.x + _bounds.width &&
        y >= _bounds.y && y <= _bounds.y + _bounds.height) {
        if (_onClick) _onClick();
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// TextInputWidget
// ---------------------------------------------------------------------------

TextInputWidget::TextInputWidget(const std::string& id) : Widget(id) {}

void TextInputWidget::AppendChar(char c) {
    if (static_cast<int>(_text.size()) < _maxLength) {
        _text += c;
        if (_onChange) _onChange(_text);
    }
}

void TextInputWidget::Backspace() {
    if (!_text.empty()) {
        _text.pop_back();
        if (_onChange) _onChange(_text);
    }
}

void TextInputWidget::Render(std::vector<DrawCommand>& cmds) const {
    if (!_visible) return;

    DrawCommand bg;
    bg.type  = DrawCommandType::FilledRect;
    bg.rect  = _bounds;
    bg.color = _focused ? Color(0.2f, 0.2f, 0.4f, 0.9f)
                        : Color(0.1f, 0.1f, 0.1f, 0.8f);
    cmds.push_back(bg);

    DrawCommand txt;
    txt.type  = DrawCommandType::Text;
    txt.rect  = Rect{_bounds.x + 4.0f, _bounds.y + 4.0f, 0.0f, 0.0f};
    txt.text  = _text.empty() ? _placeholder : _text;
    txt.color = _text.empty() ? Color::Gray() : Color::White();
    cmds.push_back(txt);

    Widget::Render(cmds);
}

bool TextInputWidget::HandleClick(float x, float y) {
    if (!_visible || !_enabled) return false;
    bool hit = (x >= _bounds.x && x <= _bounds.x + _bounds.width &&
                y >= _bounds.y && y <= _bounds.y + _bounds.height);
    _focused = hit;
    return hit;
}

// ---------------------------------------------------------------------------
// TreeViewWidget
// ---------------------------------------------------------------------------

TreeViewWidget::TreeViewWidget(const std::string& id) : Widget(id) {}

void TreeViewWidget::Render(std::vector<DrawCommand>& cmds) const {
    if (!_visible) return;
    float offsetY = _bounds.y;
    RenderNodes(_nodes, cmds, offsetY, 0);
    Widget::Render(cmds);
}

void TreeViewWidget::RenderNodes(const std::vector<TreeViewNode>& nodes,
                                  std::vector<DrawCommand>& cmds,
                                  float& offsetY, int depth) const {
    float indent = depth * 12.0f;
    for (const auto& node : nodes) {
        DrawCommand row;
        row.type  = DrawCommandType::Text;
        row.rect  = Rect{_bounds.x + indent + 4.0f, offsetY + 4.0f, 0.0f, 0.0f};
        row.text  = (node.expanded ? "v " : "> ") + node.label;
        row.color = Color::White();
        cmds.push_back(row);
        offsetY += _rowHeight;

        if (node.expanded) {
            RenderNodes(node.children, cmds, offsetY, depth + 1);
        }
    }
}

bool TreeViewWidget::HandleClick(float x, float y) {
    if (!_visible || !_enabled) return false;
    if (x < _bounds.x || x > _bounds.x + _bounds.width) return false;

    float offsetY = _bounds.y;
    for (auto& node : _nodes) {
        if (y >= offsetY && y < offsetY + _rowHeight) {
            node.expanded = !node.expanded;
            if (_onSelect) _onSelect(node.label);
            return true;
        }
        offsetY += _rowHeight;
    }
    return false;
}

} // namespace subspace
