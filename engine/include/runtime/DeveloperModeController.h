#pragma once

#include "developer/DeveloperCommandBridge.h"
#include "developer/DeveloperEditingLayer.h"

#include <string>
#include <vector>

namespace subspace {

class RuntimeWorld;

class DeveloperModeController {
public:
    DeveloperModeController();

    void InitializeDefaults();
    void SetEnabled(bool enabled);
    void Toggle();
    bool IsEnabled() const { return _enabled; }

    void Tick(float deltaSeconds);

    DeveloperCommandExecution ExecuteCommandLine(const std::string& line);
    RuntimeEditResult ExecuteCommand(const RuntimeEditCommand& command);

    DeveloperEditingLayer& GetEditingLayer() { return _editingLayer; }
    const DeveloperEditingLayer& GetEditingLayer() const { return _editingLayer; }
    DeveloperCommandBridge& GetCommandBridge() { return _commandBridge; }
    const DeveloperCommandBridge& GetCommandBridge() const { return _commandBridge; }

    std::vector<std::string> GetSupportedCommands() const;

private:
    RuntimeEditResult ExecuteFromBridge(const RuntimeEditCommand& command);
    void RefreshSupportedCommands();

    bool _enabled = false;
    bool _initializedDefaults = false;
    DeveloperEditingLayer _editingLayer;
    DeveloperCommandBridge _commandBridge;
};

} // namespace subspace
