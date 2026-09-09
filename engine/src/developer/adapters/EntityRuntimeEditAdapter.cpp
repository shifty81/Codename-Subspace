#include "developer/adapters/EntityRuntimeEditAdapter.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace subspace {
namespace {
const std::vector<std::string>& Commands()
{
    static const std::vector<std::string> commands = {
        "entity.select",
        "entity.inspect",
        "entity.spawn",
        "entity.delete",
        "entity.activate",
        "entity.deactivate",
        "entity.component.set",
        "entity.component.reset",
        "entity.component.add",
        "entity.component.remove",
        "entity.validate"
    };
    return commands;
}
}

bool EntityRuntimeEditAdapter::CanHandle(const RuntimeEditCommand& command) const
{
    const auto& commands = Commands();
    return std::find(commands.begin(), commands.end(), command.name) != commands.end();
}

RuntimeEditResult EntityRuntimeEditAdapter::Apply(const RuntimeEditCommand& command)
{
    const auto found = _handlers.find(command.name);
    if (found != _handlers.end()) {
        return found->second(command);
    }
    return ApplyDefault(command);
}

std::vector<std::string> EntityRuntimeEditAdapter::GetSupportedCommands() const
{
    return Commands();
}

void EntityRuntimeEditAdapter::RegisterHandler(std::string commandName, EntityHandler handler)
{
    if (!commandName.empty() && handler) {
        _handlers[std::move(commandName)] = std::move(handler);
    }
}

void EntityRuntimeEditAdapter::ClearHandlers()
{
    _handlers.clear();
}

RuntimeEditResult EntityRuntimeEditAdapter::ApplyDefault(const RuntimeEditCommand& command)
{
    const std::string id = command.GetArg("id", command.GetArg("entity", _selectedEntityId));
    std::ostringstream message;

    if (command.name == "entity.select") {
        if (id.empty()) {
            return RuntimeEditResult::Failure(command, "entity.select requires id=...");
        }
        _selectedEntityId = id;
        message << "Selected runtime entity '" << _selectedEntityId << "'.";
        return RuntimeEditResult::Success(command, message.str(), false);
    }

    if (id.empty() && command.name != "entity.spawn") {
        return RuntimeEditResult::Failure(command, command.name + " requires id=... or a selected entity.");
    }

    RuntimeEditResult result = RuntimeEditResult::Success(command, {}, command.name != "entity.inspect" && command.name != "entity.validate");
    message << command.name << " accepted";
    if (!id.empty()) {
        message << " for entity '" << id << "'";
    }
    message << ".";
    result.message = message.str();
    result.warnings.push_back("Default entity adapter is a safe bridge only; register concrete ECS/component handlers for real mutation.");
    return result;
}

} // namespace subspace
