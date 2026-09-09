#include "developer/adapters/ShipRuntimeEditAdapter.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace subspace {
namespace {
const std::vector<std::string>& Commands()
{
    static const std::vector<std::string> commands = {
        "ship.select",
        "ship.block.place",
        "ship.block.remove",
        "ship.block.paint",
        "ship.validate",
        "ship.rebuild",
        "ship.promote_to_blueprint"
    };
    return commands;
}

bool HasBlockCoordinates(const RuntimeEditCommand& command)
{
    return command.HasArg("x") && command.HasArg("y") && command.HasArg("z");
}
}

bool ShipRuntimeEditAdapter::CanHandle(const RuntimeEditCommand& command) const
{
    const auto& commands = Commands();
    return std::find(commands.begin(), commands.end(), command.name) != commands.end();
}

RuntimeEditResult ShipRuntimeEditAdapter::Apply(const RuntimeEditCommand& command)
{
    const auto found = _handlers.find(command.name);
    if (found != _handlers.end()) {
        return found->second(command);
    }
    return ApplyDefault(command);
}

std::vector<std::string> ShipRuntimeEditAdapter::GetSupportedCommands() const
{
    return Commands();
}

void ShipRuntimeEditAdapter::RegisterHandler(std::string commandName, ShipHandler handler)
{
    if (!commandName.empty() && handler) {
        _handlers[std::move(commandName)] = std::move(handler);
    }
}

void ShipRuntimeEditAdapter::ClearHandlers()
{
    _handlers.clear();
}

RuntimeEditResult ShipRuntimeEditAdapter::ApplyDefault(const RuntimeEditCommand& command)
{
    if (command.name == "ship.select") {
        const std::string id = command.GetArg("id", command.GetArg("ship"));
        if (id.empty()) {
            return RuntimeEditResult::Failure(command, "ship.select requires id=... or ship=...");
        }
        _activeShipId = id;
        return RuntimeEditResult::Success(command, "Selected runtime ship '" + _activeShipId + "'.", false);
    }

    if (command.name == "ship.validate" || command.name == "ship.rebuild" || command.name == "ship.promote_to_blueprint") {
        return RequireShipCommand(command, false);
    }

    if ((command.name == "ship.block.place" || command.name == "ship.block.remove" || command.name == "ship.block.paint") &&
        !HasBlockCoordinates(command)) {
        return RuntimeEditResult::Failure(command, command.name + " requires x=... y=... z=...");
    }

    return RequireShipCommand(command, true);
}

RuntimeEditResult ShipRuntimeEditAdapter::RequireShipCommand(const RuntimeEditCommand& command, bool undoable)
{
    const std::string ship = command.GetArg("ship", command.GetArg("id", _activeShipId));
    if (ship.empty()) {
        return RuntimeEditResult::Failure(command, command.name + " requires ship=... or a selected ship.");
    }

    std::ostringstream message;
    message << command.name << " accepted for ship '" << ship << "'";
    if (command.HasArg("x")) {
        message << " at [" << command.GetArg("x") << "," << command.GetArg("y") << "," << command.GetArg("z") << "]";
    }
    message << ".";

    RuntimeEditResult result = RuntimeEditResult::Success(command, message.str(), undoable);
    result.warnings.push_back("Default ship adapter is a safe bridge only; connect ShipEditorController for concrete live block mutation.");
    return result;
}

} // namespace subspace
