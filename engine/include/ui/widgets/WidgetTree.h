#pragma once

#include "ui/widgets/Widget.h"

#include <memory>
#include <string>
#include <vector>

namespace subspace {

/// Owns the root widget and drives the widget hierarchy.
///
/// Usage:
/// @code
///   WidgetTree tree;
///   auto root = std::make_shared<Widget>("root");
///   tree.SetRoot(root);
///
///   // Each frame:
///   tree.Update(dt);
///   tree.HandleClick(mouseX, mouseY);
///   tree.Render(drawCommands);
/// @endcode
class WidgetTree {
public:
    void SetRoot(WidgetPtr root);

    WidgetPtr GetRoot() const noexcept { return _root; }

    /// Find a widget anywhere in the tree by id (BFS).
    WidgetPtr Find(const std::string& id) const;

    /// Advance all widgets.
    void Update(float dt);

    /// Dispatch a click event.  Returns true if consumed.
    bool HandleClick(float x, float y);

    /// Collect draw commands for all visible widgets.
    void Render(std::vector<DrawCommand>& cmds) const;

private:
    WidgetPtr _root;
};

} // namespace subspace
