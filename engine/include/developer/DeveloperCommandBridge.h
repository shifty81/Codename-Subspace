#pragma once

#include "developer/RuntimeEditCommand.h"

#include <functional>
#include <string>
#include <vector>

namespace subspace {

struct DeveloperCommandExecution {
    bool accepted = false;
    bool success = false;
    std::string line;
    std::string message;
    RuntimeEditCommand command;
    RuntimeEditResult result;
};

class DeveloperCommandBridge {
public:
    using CommandExecutor = std::function<RuntimeEditResult(const RuntimeEditCommand&)>;

    DeveloperCommandBridge() = default;
    explicit DeveloperCommandBridge(CommandExecutor executor);

    void SetExecutor(CommandExecutor executor);
    void SetSupportedCommands(std::vector<std::string> commands);

    DeveloperCommandExecution ExecuteLine(const std::string& line) const;
    RuntimeEditCommand ParseLine(const std::string& line) const;
    std::vector<std::string> CompletePrefix(const std::string& prefix) const;
    const std::vector<std::string>& GetSupportedCommands() const { return _supportedCommands; }

private:
    static std::vector<std::string> Tokenize(const std::string& line);
    static void ApplyToken(RuntimeEditCommand& command, const std::string& token, int positionalIndex);

    CommandExecutor _executor;
    std::vector<std::string> _supportedCommands;
};

} // namespace subspace
