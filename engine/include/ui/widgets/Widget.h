#pragma once

#include "ui/UITypes.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace subspace {

// Forward declaration
class Widget;
using WidgetPtr = std::shared_ptr<Widget>;

/// Base class for all composable UI widgets in the new widget framework.
///
/// The widget framework sits alongside the existing UIPanel / UIElement
/// system.  It adds parent/child hierarchy, per-widget layout, and a
/// separate Render(DrawCommand&) callback model.
///
/// Widgets are NOT IComponents — they live in a WidgetTree, not the ECS.
class Widget {
public:
    explicit Widget(const std::string& id = "");
    virtual ~Widget() = default;

    // ---- Identity ----------------------------------------------------------
    const std::string& GetId()       const { return _id; }
    void               SetId(const std::string& id) { _id = id; }

    // ---- Layout ------------------------------------------------------------
    float GetX()      const { return _bounds.x; }
    float GetY()      const { return _bounds.y; }
    float GetWidth()  const { return _bounds.width; }
    float GetHeight() const { return _bounds.height; }

    void SetPosition(float x, float y) { _bounds.x = x; _bounds.y = y; }
    void SetSize(float w, float h)     { _bounds.width = w; _bounds.height = h; }
    void SetBounds(const Rect& r)      { _bounds = r; }
    const Rect& GetBounds()            const { return _bounds; }

    // ---- Visibility & interaction ------------------------------------------
    bool IsVisible()   const { return _visible; }
    void SetVisible(bool v)  { _visible = v; }

    bool IsEnabled()   const { return _enabled; }
    void SetEnabled(bool e)  { _enabled = e; }

    // ---- Hierarchy ---------------------------------------------------------
    void        AddChild(WidgetPtr child);
    bool        RemoveChild(const std::string& childId);
    WidgetPtr   FindChild(const std::string& childId) const;
    const std::vector<WidgetPtr>& GetChildren() const { return _children; }
    Widget*     GetParent() const { return _parent; }
    std::size_t GetChildCount() const { return _children.size(); }

    // ---- Per-frame lifecycle -----------------------------------------------
    virtual void Update(float dt);
    virtual void Render(std::vector<DrawCommand>& cmds) const;

    /// Process a click.  Returns true if consumed by this widget or a child.
    virtual bool HandleClick(float x, float y);

protected:
    std::string            _id;
    Rect                   _bounds{};
    bool                   _visible = true;
    bool                   _enabled = true;
    Widget*                _parent  = nullptr;
    std::vector<WidgetPtr> _children;
};

} // namespace subspace
