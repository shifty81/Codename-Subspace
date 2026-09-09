#pragma once

#include "developer/DeveloperCommandBridge.h"
#include "developer/ui/DeveloperCommandHistory.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace subspace {

struct DeveloperConsoleLine {
    std::string text;
    bool isError = false;
};

class DeveloperConsole {
public:
    using CommandLineExecutor = std::function<DeveloperCommandExecution(const std::string&)>;

    explicit DeveloperConsole(std::size_t maxOutputLines = 96);

    void SetExecutor(CommandLineExecutor executor);
    DeveloperCommandExecution Submit(const std::string& line);

    void SetInput(std::string input);
    const std::string& GetInput() const { return _input; }
    void AppendCharacter(char ch);
    void Backspace();
    void ClearInput();

    void AddOutput(std::string text, bool isError = false);
    void ClearOutput();

    DeveloperCommandHistory& GetHistory() { return _history; }
    const DeveloperCommandHistory& GetHistory() const { return _history; }
    const std::vector<DeveloperConsoleLine>& GetOutputLines() const { return _outputLines; }
    const DeveloperCommandExecution& GetLastExecution() const { return _lastExecution; }

private:
    void TrimOutput();

    std::size_t _maxOutputLines = 96;
    std::string _input;
    DeveloperCommandHistory _history;
    std::vector<DeveloperConsoleLine> _outputLines;
    CommandLineExecutor _executor;
    DeveloperCommandExecution _lastExecution;
};

} // namespace subspace
