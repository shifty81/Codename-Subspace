#pragma once

#include "ui/SubspaceUiFramework.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct RegisteredEditorAction {
    EditorContextAction action;
    std::string category;
    std::function<bool()> execute;
};

class EditorActionRegistry {
public:
    void Register(RegisteredEditorAction action);
    const RegisteredEditorAction* Find(const std::string& id) const;
    std::vector<EditorContextAction> ActionsForCategory(const std::string& category) const;
    bool Execute(const std::string& id, std::string* reason = nullptr) const;
private:
    std::unordered_map<std::string, RegisteredEditorAction> actions_;
};

} // namespace subspace
