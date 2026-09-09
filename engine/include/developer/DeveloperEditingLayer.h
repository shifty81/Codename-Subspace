#pragma once

#include "developer/AssetHotReloadService.h"
#include "developer/RuntimeEditSession.h"
#include "developer/adapters/IRuntimeEditAdapter.h"

#include <memory>
#include <string>
#include <vector>

namespace subspace {

class DeveloperEditingLayer {
public:
    DeveloperEditingLayer();

    void BeginSession(std::string label = "Development Play Session");
    void EndSession();
    void Tick(float deltaSeconds);

    RuntimeEditResult Execute(const RuntimeEditCommand& command);
    RuntimeEditResult ExecuteWithoutRecording(const RuntimeEditCommand& command);

    void RegisterAdapter(std::unique_ptr<IRuntimeEditAdapter> adapter);
    void RegisterDefaultAdapters();
    void ClearAdapters();

    std::vector<std::string> GetSupportedCommands() const;
    const RuntimeEditResult& GetLastResult() const { return _lastResult; }

    RuntimeEditSession& GetSession() { return _session; }
    const RuntimeEditSession& GetSession() const { return _session; }
    AssetHotReloadService& GetHotReloadService() { return _hotReloadService; }
    const AssetHotReloadService& GetHotReloadService() const { return _hotReloadService; }

private:
    RuntimeEditResult DispatchToAdapter(const RuntimeEditCommand& command);
    RuntimeEditResult HandleBuiltInCommand(const RuntimeEditCommand& command);

    RuntimeEditSession _session;
    AssetHotReloadService _hotReloadService;
    std::vector<std::unique_ptr<IRuntimeEditAdapter>> _adapters;
    RuntimeEditResult _lastResult;
};

} // namespace subspace
