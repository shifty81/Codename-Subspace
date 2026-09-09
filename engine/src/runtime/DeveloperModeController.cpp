#include "runtime/DeveloperModeController.h"

#include <algorithm>

namespace subspace {

DeveloperModeController::DeveloperModeController()
{
    _commandBridge.SetExecutor([this](const RuntimeEditCommand& command) {
        return ExecuteFromBridge(command);
    });
    InitializeDefaults();
}

void DeveloperModeController::InitializeDefaults()
{
    if (_initializedDefaults) {
        return;
    }
    _editingLayer.RegisterDefaultAdapters();
    _initializedDefaults = true;
    RefreshSupportedCommands();
}

void DeveloperModeController::SetEnabled(bool enabled)
{
    _enabled = enabled;
}

void DeveloperModeController::Toggle()
{
    _enabled = !_enabled;
}

void DeveloperModeController::Tick(float deltaSeconds)
{
    if (_enabled) {
        _editingLayer.Tick(deltaSeconds);
    }
}

DeveloperCommandExecution DeveloperModeController::ExecuteCommandLine(const std::string& line)
{
    DeveloperCommandExecution modeExecution;
    modeExecution.line = line;

    RuntimeEditCommand parsed = _commandBridge.ParseLine(line);
    if (parsed.name == "dev.mode.toggle") {
        Toggle();
        modeExecution.accepted = true;
        modeExecution.success = true;
        modeExecution.command = parsed;
        modeExecution.message = _enabled ? "Developer mode enabled." : "Developer mode disabled.";
        modeExecution.result = RuntimeEditResult::Success(parsed, modeExecution.message, false);
        return modeExecution;
    }
    if (parsed.name == "dev.mode.on") {
        SetEnabled(true);
        modeExecution.accepted = true;
        modeExecution.success = true;
        modeExecution.command = parsed;
        modeExecution.message = "Developer mode enabled.";
        modeExecution.result = RuntimeEditResult::Success(parsed, modeExecution.message, false);
        return modeExecution;
    }
    if (parsed.name == "dev.mode.off") {
        SetEnabled(false);
        modeExecution.accepted = true;
        modeExecution.success = true;
        modeExecution.command = parsed;
        modeExecution.message = "Developer mode disabled.";
        modeExecution.result = RuntimeEditResult::Success(parsed, modeExecution.message, false);
        return modeExecution;
    }

    return _commandBridge.ExecuteLine(line);
}

RuntimeEditResult DeveloperModeController::ExecuteCommand(const RuntimeEditCommand& command)
{
    return ExecuteFromBridge(command);
}

std::vector<std::string> DeveloperModeController::GetSupportedCommands() const
{
    std::vector<std::string> commands = _editingLayer.GetSupportedCommands();
    commands.push_back("dev.mode.toggle");
    commands.push_back("dev.mode.on");
    commands.push_back("dev.mode.off");
    std::sort(commands.begin(), commands.end());
    commands.erase(std::unique(commands.begin(), commands.end()), commands.end());
    return commands;
}

RuntimeEditResult DeveloperModeController::ExecuteFromBridge(const RuntimeEditCommand& command)
{
    if (!_enabled && command.name.rfind("dev.", 0) != 0) {
        return RuntimeEditResult::Failure(command, "Developer mode is disabled. Run dev.mode.on or dev.mode.toggle first.");
    }
    return _editingLayer.Execute(command);
}

void DeveloperModeController::RefreshSupportedCommands()
{
    _commandBridge.SetSupportedCommands(GetSupportedCommands());
}

} // namespace subspace
