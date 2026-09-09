#pragma once

#include "developer/adapters/IRuntimeEditAdapter.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

class ShipRuntimeEditAdapter final : public IRuntimeEditAdapter {
public:
    using ShipHandler = std::function<RuntimeEditResult(const RuntimeEditCommand&)>;

    std::string GetName() const override { return "ShipRuntimeEditAdapter"; }
    bool CanHandle(const RuntimeEditCommand& command) const override;
    RuntimeEditResult Apply(const RuntimeEditCommand& command) override;
    std::vector<std::string> GetSupportedCommands() const override;

    void RegisterHandler(std::string commandName, ShipHandler handler);
    void ClearHandlers();
    const std::string& GetActiveShipId() const { return _activeShipId; }

private:
    RuntimeEditResult ApplyDefault(const RuntimeEditCommand& command);
    RuntimeEditResult RequireShipCommand(const RuntimeEditCommand& command, bool undoable);

    std::string _activeShipId;
    std::unordered_map<std::string, ShipHandler> _handlers;
};

} // namespace subspace
