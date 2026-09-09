#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct DeveloperInputAction {
    std::string actionId;
    std::string displayName;
    std::string commandLine;
};

class DeveloperInputRouter {
public:
    void BindAction(DeveloperInputAction action);
    bool HasAction(const std::string& actionId) const;
    std::string ResolveCommand(const std::string& actionId) const;
    std::vector<DeveloperInputAction> GetActions() const;
    void BindDefaultActions();
    void Clear();

private:
    std::unordered_map<std::string, DeveloperInputAction> _actions;
};

} // namespace subspace
