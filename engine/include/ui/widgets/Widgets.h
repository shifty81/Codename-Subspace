#pragma once

#include "ui/widgets/Widget.h"

#include <functional>
#include <string>

namespace subspace {

/// A clickable button widget with an optional label and click callback.
class ButtonWidget : public Widget {
public:
    explicit ButtonWidget(const std::string& id = "",
                          const std::string& label = "");

    const std::string& GetLabel() const { return _label; }
    void SetLabel(const std::string& l)  { _label = l; }

    const Color& GetBackgroundColor() const { return _bgColor; }
    void SetBackgroundColor(const Color& c)  { _bgColor = c; }

    const Color& GetTextColor() const { return _textColor; }
    void SetTextColor(const Color& c)  { _textColor = c; }

    using ClickCallback = std::function<void()>;
    void SetOnClick(ClickCallback cb) { _onClick = std::move(cb); }

    void Render(std::vector<DrawCommand>& cmds) const override;
    bool HandleClick(float x, float y)          override;

private:
    std::string   _label;
    Color         _bgColor   = Color::DarkGray();
    Color         _textColor = Color::White();
    ClickCallback _onClick;
};

/// A single-line text input widget.
class TextInputWidget : public Widget {
public:
    explicit TextInputWidget(const std::string& id = "");

    const std::string& GetText()  const { return _text; }
    void SetText(const std::string& t)   { _text = t; }

    const std::string& GetPlaceholder() const { return _placeholder; }
    void SetPlaceholder(const std::string& p)  { _placeholder = p; }

    bool IsFocused() const { return _focused; }
    void SetFocused(bool f) { _focused = f; }

    int GetMaxLength() const { return _maxLength; }
    void SetMaxLength(int n)  { _maxLength = n; }

    using ChangeCallback = std::function<void(const std::string&)>;
    void SetOnChange(ChangeCallback cb) { _onChange = std::move(cb); }

    /// Append a character (e.g. from platform keyboard events).
    void AppendChar(char c);

    /// Delete the last character.
    void Backspace();

    void Render(std::vector<DrawCommand>& cmds) const override;
    bool HandleClick(float x, float y)          override;

private:
    std::string    _text;
    std::string    _placeholder;
    bool           _focused   = false;
    int            _maxLength = 256;
    ChangeCallback _onChange;
};

/// A node in a TreeViewWidget.
struct TreeViewNode {
    std::string              label;
    bool                     expanded = false;
    std::vector<TreeViewNode> children;
};

/// A collapsible tree view widget.
class TreeViewWidget : public Widget {
public:
    explicit TreeViewWidget(const std::string& id = "");

    void AddNode(TreeViewNode node) { _nodes.push_back(std::move(node)); }
    void Clear() { _nodes.clear(); }

    const std::vector<TreeViewNode>& GetNodes() const { return _nodes; }

    float GetRowHeight() const { return _rowHeight; }
    void SetRowHeight(float h)  { _rowHeight = h; }

    using SelectCallback = std::function<void(const std::string& label)>;
    void SetOnSelect(SelectCallback cb) { _onSelect = std::move(cb); }

    void Render(std::vector<DrawCommand>& cmds) const override;
    bool HandleClick(float x, float y)          override;

private:
    void RenderNodes(const std::vector<TreeViewNode>& nodes,
                     std::vector<DrawCommand>& cmds,
                     float& offsetY, int depth) const;

    std::vector<TreeViewNode> _nodes;
    float          _rowHeight = 20.0f;
    SelectCallback _onSelect;
};

} // namespace subspace
