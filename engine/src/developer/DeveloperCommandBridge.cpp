#include "developer/DeveloperCommandBridge.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>

namespace subspace {

DeveloperCommandBridge::DeveloperCommandBridge(CommandExecutor executor)
    : _executor(std::move(executor))
{
}

void DeveloperCommandBridge::SetExecutor(CommandExecutor executor)
{
    _executor = std::move(executor);
}

void DeveloperCommandBridge::SetSupportedCommands(std::vector<std::string> commands)
{
    std::sort(commands.begin(), commands.end());
    commands.erase(std::unique(commands.begin(), commands.end()), commands.end());
    _supportedCommands = std::move(commands);
}

DeveloperCommandExecution DeveloperCommandBridge::ExecuteLine(const std::string& line) const
{
    DeveloperCommandExecution execution;
    execution.line = line;

    RuntimeEditCommand command = ParseLine(line);
    execution.command = command;

    if (command.name.empty()) {
        execution.message = "No command entered.";
        return execution;
    }

    if (!_executor) {
        execution.accepted = true;
        execution.message = "DeveloperCommandBridge has no executor.";
        execution.result = RuntimeEditResult::Failure(command, execution.message);
        return execution;
    }

    RuntimeEditResult result = _executor(command);
    execution.accepted = result.handled;
    execution.success = result.success;
    execution.message = result.message;
    execution.result = result;
    return execution;
}

RuntimeEditCommand DeveloperCommandBridge::ParseLine(const std::string& line) const
{
    RuntimeEditCommand command;
    command.source = "developer-command-bridge";

    const std::vector<std::string> tokens = Tokenize(line);
    if (tokens.empty()) {
        return command;
    }

    command.name = tokens.front();
    for (std::size_t i = 1; i < tokens.size(); ++i) {
        ApplyToken(command, tokens[i], static_cast<int>(i));
    }
    return command;
}

std::vector<std::string> DeveloperCommandBridge::CompletePrefix(const std::string& prefix) const
{
    std::vector<std::string> matches;
    for (const std::string& command : _supportedCommands) {
        if (command.rfind(prefix, 0) == 0) {
            matches.push_back(command);
        }
    }
    return matches;
}

std::vector<std::string> DeveloperCommandBridge::Tokenize(const std::string& line)
{
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (char ch : line) {
        if ((ch == '"' || ch == '\'') && (!inQuotes || ch == quoteChar)) {
            if (inQuotes) {
                inQuotes = false;
                quoteChar = '\0';
            } else {
                inQuotes = true;
                quoteChar = ch;
            }
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

void DeveloperCommandBridge::ApplyToken(RuntimeEditCommand& command, const std::string& token, int positionalIndex)
{
    const std::size_t equals = token.find('=');
    if (equals != std::string::npos) {
        const std::string key = token.substr(0, equals);
        const std::string value = token.substr(equals + 1);
        command.SetArg(key, value);
        return;
    }

    command.SetArg("arg" + std::to_string(positionalIndex), token);
}

} // namespace subspace
