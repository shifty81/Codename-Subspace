#include "ui/widgets/Widget.h"

#include <algorithm>

namespace subspace {

Widget::Widget(const std::string& id) : _id(id) {}

void Widget::AddChild(WidgetPtr child) {
    if (!child) return;
    child->_parent = this;
    _children.push_back(std::move(child));
}

bool Widget::RemoveChild(const std::string& childId) {
    auto it = std::find_if(_children.begin(), _children.end(),
                           [&](const WidgetPtr& w){ return w && w->_id == childId; });
    if (it == _children.end()) return false;
    (*it)->_parent = nullptr;
    _children.erase(it);
    return true;
}

WidgetPtr Widget::FindChild(const std::string& childId) const {
    for (const auto& c : _children) {
        if (!c) continue;
        if (c->_id == childId) return c;
        auto found = c->FindChild(childId);
        if (found) return found;
    }
    return nullptr;
}

void Widget::Update(float dt) {
    if (!_visible) return;
    for (auto& c : _children)
        if (c) c->Update(dt);
}

void Widget::Render(std::vector<DrawCommand>& cmds) const {
    if (!_visible) return;
    for (const auto& c : _children)
        if (c) c->Render(cmds);
}

bool Widget::HandleClick(float x, float y) {
    if (!_visible || !_enabled) return false;
    // Propagate to children in reverse order (topmost first).
    for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
        if (*it && (*it)->HandleClick(x, y)) return true;
    }
    return false;
}

} // namespace subspace
