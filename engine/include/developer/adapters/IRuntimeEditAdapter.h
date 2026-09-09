#pragma once

#include "developer/RuntimeEditCommand.h"

#include <string>
#include <vector>

namespace subspace {

class IRuntimeEditAdapter {
public:
    virtual ~IRuntimeEditAdapter() = default;

    virtual std::string GetName() const = 0;
    virtual bool CanHandle(const RuntimeEditCommand& command) const = 0;
    virtual RuntimeEditResult Apply(const RuntimeEditCommand& command) = 0;
    virtual std::vector<std::string> GetSupportedCommands() const = 0;
};

} // namespace subspace
