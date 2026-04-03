#include "ui/widgets/WidgetTree.h"

#include <queue>

namespace subspace {

void WidgetTree::SetRoot(WidgetPtr root) {
    _root = std::move(root);
}

WidgetPtr WidgetTree::Find(const std::string& id) const {
    if (!_root) return nullptr;
    if (_root->GetId() == id) return _root;
    return _root->FindChild(id);
}

void WidgetTree::Update(float dt) {
    if (_root) _root->Update(dt);
}

bool WidgetTree::HandleClick(float x, float y) {
    return _root ? _root->HandleClick(x, y) : false;
}

void WidgetTree::Render(std::vector<DrawCommand>& cmds) const {
    if (_root) _root->Render(cmds);
}

} // namespace subspace
