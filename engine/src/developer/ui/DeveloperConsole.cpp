#include "developer/ui/DeveloperConsole.h"

#include <utility>

namespace subspace {

DeveloperConsole::DeveloperConsole(std::size_t maxOutputLines)
    : _maxOutputLines(maxOutputLines == 0 ? 1 : maxOutputLines)
{
}

void DeveloperConsole::SetExecutor(CommandLineExecutor executor)
{
    _executor = std::move(executor);
}

DeveloperCommandExecution DeveloperConsole::Submit(const std::string& line)
{
    if (line.empty()) {
        DeveloperCommandExecution execution;
        execution.line = line;
        execution.message = "No command entered.";
        _lastExecution = execution;
        return _lastExecution;
    }

    _history.Push(line);
    AddOutput("> " + line, false);

    if (!_executor) {
        DeveloperCommandExecution execution;
        execution.line = line;
        execution.message = "Developer console has no command executor.";
        AddOutput(execution.message, true);
        _lastExecution = execution;
        return _lastExecution;
    }

    DeveloperCommandExecution execution = _executor(line);
    AddOutput(execution.message.empty() ? "Command completed." : execution.message, !execution.success);
    _lastExecution = execution;
    ClearInput();
    return _lastExecution;
}

void DeveloperConsole::SetInput(std::string input)
{
    _input = std::move(input);
}

void DeveloperConsole::AppendCharacter(char ch)
{
    _input.push_back(ch);
}

void DeveloperConsole::Backspace()
{
    if (!_input.empty()) {
        _input.pop_back();
    }
}

void DeveloperConsole::ClearInput()
{
    _input.clear();
}

void DeveloperConsole::AddOutput(std::string text, bool isError)
{
    DeveloperConsoleLine line;
    line.text = std::move(text);
    line.isError = isError;
    _outputLines.push_back(std::move(line));
    TrimOutput();
}

void DeveloperConsole::ClearOutput()
{
    _outputLines.clear();
}

void DeveloperConsole::TrimOutput()
{
    while (_outputLines.size() > _maxOutputLines) {
        _outputLines.erase(_outputLines.begin());
    }
}

} // namespace subspace
