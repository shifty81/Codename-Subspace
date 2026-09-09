#include "developer/input/DeveloperInputRouter.h"

#include <algorithm>

namespace subspace {

void DeveloperInputRouter::BindAction(DeveloperInputAction action) {
    _actions[action.actionId] = std::move(action);
}

bool DeveloperInputRouter::HasAction(const std::string& actionId) const {
    return _actions.find(actionId) != _actions.end();
}

std::string DeveloperInputRouter::ResolveCommand(const std::string& actionId) const {
    auto it = _actions.find(actionId);
    return it == _actions.end() ? std::string{} : it->second.commandLine;
}

std::vector<DeveloperInputAction> DeveloperInputRouter::GetActions() const {
    std::vector<DeveloperInputAction> result;
    result.reserve(_actions.size());
    for (const auto& entry : _actions) {
        result.push_back(entry.second);
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.actionId < b.actionId;
    });
    return result;
}

void DeveloperInputRouter::BindDefaultActions() {
    BindAction({"toggle_console", "Toggle Developer Console", "dev.mode.toggle"});
    BindAction({"undo", "Undo Runtime Edit", "dev.undo"});
    BindAction({"redo", "Redo Runtime Edit", "dev.redo"});
    BindAction({"select_under_cursor", "Select Under Cursor", "viewport.pick cursor"});
    BindAction({"reload_selected_asset", "Reload Selected Asset", "asset.reload selected=true"});
    BindAction({"validate_selection", "Validate Selection", "entity.validate selected=true"});
}

void DeveloperInputRouter::Clear() {
    _actions.clear();
}

} // namespace subspace
