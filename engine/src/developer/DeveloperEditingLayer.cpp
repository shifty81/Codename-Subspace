#include "developer/DeveloperEditingLayer.h"

#include "developer/adapters/AssetRuntimeEditAdapter.h"
#include "developer/adapters/EntityRuntimeEditAdapter.h"
#include "developer/adapters/ShipRuntimeEditAdapter.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace subspace {

DeveloperEditingLayer::DeveloperEditingLayer()
{
    _lastResult = RuntimeEditResult::Ignored(RuntimeEditCommand{}, "No developer command has run yet.");
    BeginSession();
}

void DeveloperEditingLayer::BeginSession(std::string label)
{
    _session.Begin(std::move(label));
}

void DeveloperEditingLayer::EndSession()
{
    _session.End();
}

void DeveloperEditingLayer::Tick(float)
{
    for (const AssetHotReloadChange& change : _hotReloadService.PollChanges()) {
        if (change.modified || change.appeared) {
            RuntimeEditCommand command;
            command.name = "asset.reload";
            command.source = "asset-hot-reload-service";
            command.SetArg("id", change.assetId);
            command.SetArg("path", change.path.string());
            command.SetArg("kind", change.kind);
            Execute(command);
        }
    }
}

RuntimeEditResult DeveloperEditingLayer::Execute(const RuntimeEditCommand& command)
{
    RuntimeEditCommand sequenced = command;
    if (sequenced.sequence == 0) {
        sequenced.sequence = _session.NextSequence();
    }

    RuntimeEditResult result = HandleBuiltInCommand(sequenced);
    if (!result.handled) {
        result = DispatchToAdapter(sequenced);
    }

    const bool isHistoryNavigation = sequenced.name == "dev.undo" || sequenced.name == "dev.redo";
    if (!isHistoryNavigation && result.handled && result.success) {
        _session.RecordApplied(result);
    }

    _lastResult = result;
    return result;
}

RuntimeEditResult DeveloperEditingLayer::ExecuteWithoutRecording(const RuntimeEditCommand& command)
{
    RuntimeEditResult result = HandleBuiltInCommand(command);
    if (!result.handled) {
        result = DispatchToAdapter(command);
    }
    _lastResult = result;
    return result;
}

void DeveloperEditingLayer::RegisterAdapter(std::unique_ptr<IRuntimeEditAdapter> adapter)
{
    if (adapter) {
        _adapters.push_back(std::move(adapter));
    }
}

void DeveloperEditingLayer::RegisterDefaultAdapters()
{
    if (!_adapters.empty()) {
        return;
    }
    RegisterAdapter(std::make_unique<ShipRuntimeEditAdapter>());
    RegisterAdapter(std::make_unique<AssetRuntimeEditAdapter>(&_hotReloadService));
    RegisterAdapter(std::make_unique<EntityRuntimeEditAdapter>());
}

void DeveloperEditingLayer::ClearAdapters()
{
    _adapters.clear();
}

std::vector<std::string> DeveloperEditingLayer::GetSupportedCommands() const
{
    std::vector<std::string> commands = {
        "dev.undo",
        "dev.redo",
        "dev.session.begin",
        "dev.session.end",
        "dev.session.clear"
    };

    for (const auto& adapter : _adapters) {
        std::vector<std::string> adapterCommands = adapter->GetSupportedCommands();
        commands.insert(commands.end(), adapterCommands.begin(), adapterCommands.end());
    }

    std::sort(commands.begin(), commands.end());
    commands.erase(std::unique(commands.begin(), commands.end()), commands.end());
    return commands;
}

RuntimeEditResult DeveloperEditingLayer::DispatchToAdapter(const RuntimeEditCommand& command)
{
    for (const auto& adapter : _adapters) {
        if (adapter->CanHandle(command)) {
            return adapter->Apply(command);
        }
    }
    return RuntimeEditResult::Ignored(command, "No runtime edit adapter can handle command '" + command.name + "'.");
}

RuntimeEditResult DeveloperEditingLayer::HandleBuiltInCommand(const RuntimeEditCommand& command)
{
    if (command.name == "dev.undo") {
        return _session.Undo([this](const RuntimeEditCommand& undoCommand) {
            return ExecuteWithoutRecording(undoCommand);
        });
    }
    if (command.name == "dev.redo") {
        return _session.Redo([this](const RuntimeEditCommand& redoCommand) {
            return ExecuteWithoutRecording(redoCommand);
        });
    }
    if (command.name == "dev.session.begin") {
        BeginSession(command.GetArg("label", "Development Play Session"));
        return RuntimeEditResult::Success(command, "Runtime edit session started.", false);
    }
    if (command.name == "dev.session.end") {
        EndSession();
        return RuntimeEditResult::Success(command, "Runtime edit session ended.", false);
    }
    if (command.name == "dev.session.clear") {
        _session.ClearHistory();
        return RuntimeEditResult::Success(command, "Runtime edit session history cleared.", false);
    }
    return RuntimeEditResult::Ignored(command);
}

} // namespace subspace
