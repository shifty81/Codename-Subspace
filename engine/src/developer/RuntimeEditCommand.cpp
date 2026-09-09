#include "developer/RuntimeEditCommand.h"

namespace subspace {

bool RuntimeEditCommand::HasArg(const std::string& key) const
{
    return args.find(key) != args.end();
}

std::string RuntimeEditCommand::GetArg(const std::string& key, const std::string& fallback) const
{
    const auto found = args.find(key);
    return found == args.end() ? fallback : found->second;
}

void RuntimeEditCommand::SetArg(const std::string& key, const std::string& value)
{
    args[key] = value;
}

RuntimeEditResult RuntimeEditResult::Success(RuntimeEditCommand command, std::string message, bool undoable)
{
    RuntimeEditResult result;
    result.handled = true;
    result.success = true;
    result.undoable = undoable;
    result.message = std::move(message);
    result.command = std::move(command);
    return result;
}

RuntimeEditResult RuntimeEditResult::Failure(RuntimeEditCommand command, std::string message)
{
    RuntimeEditResult result;
    result.handled = true;
    result.success = false;
    result.undoable = false;
    result.message = std::move(message);
    result.command = std::move(command);
    return result;
}

RuntimeEditResult RuntimeEditResult::Ignored(RuntimeEditCommand command, std::string message)
{
    RuntimeEditResult result;
    result.handled = false;
    result.success = false;
    result.undoable = false;
    result.message = std::move(message);
    result.command = std::move(command);
    return result;
}

} // namespace subspace
