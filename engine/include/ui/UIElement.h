#pragma once

#include "ui/UITypes.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace subspace {

/// Identifies the kind of a UIElement.
enum class UIElementType {
    Label,
    Button,
    ProgressBar,
    Panel,
    Separator,
    Image,
    Checkbox
};

// Forward declaration
class UIElement;
using UIElementPtr = std::shared_ptr<UIElement>;

/// Base class for all UI elements.
class UIElement {
public:
    explicit UIElement(UIElementType type) : _type(type) {}
    virtual ~UIElement() = default;

    UIElementType GetType()  const { return _type; }
    const std::string& GetId() const { return _id; }
    void SetId(const std::string& id) { _id = id; }

    bool IsVisible()   const { return _visible; }
    void SetVisible(bool v) { _visible = v; }

    bool IsEnabled()   const { return _enabled; }
    void SetEnabled(bool e) { _enabled = e; }

    const Rect& GetBounds() const { return _bounds; }
    void SetBounds(const Rect& r) { _bounds = r; }
    void SetPosition(float x, float y) { _bounds.x = x; _bounds.y = y; }
    void SetSize(float w, float h)     { _bounds.width = w; _bounds.height = h; }

    /// Emit draw commands for this element.
    virtual void Render(std::vector<DrawCommand>& commands) const = 0;

    /// Process a click at (x, y) in screen coordinates. Returns true if consumed.
    virtual bool HandleClick(float x, float y) { (void)x; (void)y; return false; }

protected:
    UIElementType _type;
    std::string   _id;
    bool          _visible = true;
    bool          _enabled = true;
    Rect          _bounds;
};

// --------------------------------------------------------------------------
// Concrete UI element types
// --------------------------------------------------------------------------

/// Static text label.
class UILabel : public UIElement {
public:
    UILabel() : UIElement(UIElementType::Label) {}

    const std::string& GetText()  const { return _text; }
    void SetText(const std::string& t) { _text = t; }

    const Color& GetColor() const { return _color; }
    void SetColor(const Color& c) { _color = c; }

    int GetFontSize() const { return _fontSize; }
    void SetFontSize(int s) { _fontSize = s; }

    void Render(std::vector<DrawCommand>& commands) const override;

private:
    std::string _text;
    Color       _color    = Color::White();
    int         _fontSize = 14;
};

/// Clickable button.
class UIButton : public UIElement {
public:
    UIButton() : UIElement(UIElementType::Button) {}

    const std::string& GetLabel() const { return _label; }
    void SetLabel(const std::string& l) { _label = l; }

    const Color& GetBackgroundColor() const { return _bgColor; }
    void SetBackgroundColor(const Color& c) { _bgColor = c; }

    const Color& GetTextColor() const { return _textColor; }
    void SetTextColor(const Color& c)  { _textColor = c; }

    using ClickCallback = std::function<void()>;
    void SetOnClick(ClickCallback cb) { _onClick = std::move(cb); }

    void Render(std::vector<DrawCommand>& commands) const override;
    bool HandleClick(float x, float y) override;

private:
    std::string   _label;
    Color         _bgColor   = Color::DarkGray();
    Color         _textColor = Color::White();
    ClickCallback _onClick;
};

/// Horizontal progress bar with value in [0, 1].
class UIProgressBar : public UIElement {
public:
    UIProgressBar() : UIElement(UIElementType::ProgressBar) {}

    float GetValue() const { return _value; }
    void  SetValue(float v) { _value = std::max(0.0f, std::min(1.0f, v)); }

    const Color& GetFillColor()       const { return _fillColor; }
    void SetFillColor(const Color& c)       { _fillColor = c; }

    const Color& GetBackgroundColor() const { return _bgColor; }
    void SetBackgroundColor(const Color& c) { _bgColor = c; }

    const std::string& GetLabel() const { return _label; }
    void SetLabel(const std::string& l) { _label = l; }

    /// Enable automatic red/yellow/green coloring based on value.
    bool IsAutoColor() const { return _autoColor; }
    void SetAutoColor(bool ac) { _autoColor = ac; }

    void Render(std::vector<DrawCommand>& commands) const override;

private:
    float       _value     = 0.0f;
    Color       _fillColor = Color::Green();
    Color       _bgColor   = Color(0.15f, 0.15f, 0.15f, 0.8f);
    std::string _label;
    bool        _autoColor = false;
};

/// Horizontal separator line.
class UISeparator : public UIElement {
public:
    UISeparator() : UIElement(UIElementType::Separator) {}

    const Color& GetColor() const { return _color; }
    void SetColor(const Color& c) { _color = c; }

    void Render(std::vector<DrawCommand>& commands) const override;

private:
    Color _color = Color::Gray();
};

/// Checkbox with on/off state.
class UICheckbox : public UIElement {
public:
    UICheckbox() : UIElement(UIElementType::Checkbox) {}

    bool IsChecked() const { return _checked; }
    void SetChecked(bool c) { _checked = c; }

    const std::string& GetLabel() const { return _label; }
    void SetLabel(const std::string& l) { _label = l; }

    using ChangeCallback = std::function<void(bool)>;
    void SetOnChange(ChangeCallback cb) { _onChange = std::move(cb); }

    void Render(std::vector<DrawCommand>& commands) const override;
    bool HandleClick(float x, float y) override;

private:
    bool           _checked = false;
    std::string    _label;
    ChangeCallback _onChange;
};

} // namespace subspace
