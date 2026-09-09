#pragma once

#include "developer/adapters/IRuntimeEditAdapter.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

class EntityRuntimeEditAdapter final : public IRuntimeEditAdapter {
public:
    using EntityHandler = std::function<RuntimeEditResult(const RuntimeEditCommand&)>;

    std::string GetName() const override { return "EntityRuntimeEditAdapter"; }
    bool CanHandle(const RuntimeEditCommand& command) const override;
    RuntimeEditResult Apply(const RuntimeEditCommand& command) override;
    std::vector<std::string> GetSupportedCommands() const override;

    void RegisterHandler(std::string commandName, EntityHandler handler);
    void ClearHandlers();
    const std::string& GetSelectedEntityId() const { return _selectedEntityId; }

private:
    RuntimeEditResult ApplyDefault(const RuntimeEditCommand& command);

    std::string _selectedEntityId;
    std::unordered_map<std::string, EntityHandler> _handlers;
};

} // namespace subspace
