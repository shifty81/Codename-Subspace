#pragma once

#include "developer/AssetHotReloadService.h"
#include "developer/adapters/IRuntimeEditAdapter.h"
#include "developer/assets/AssetReloadPipeline.h"
#include "developer/resources/RuntimeResourceRegistry.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

class AssetRuntimeEditAdapter final : public IRuntimeEditAdapter {
public:
    using ReloadHandler = std::function<RuntimeEditResult(const RuntimeEditCommand&)>;

    AssetRuntimeEditAdapter() = default;
    explicit AssetRuntimeEditAdapter(AssetHotReloadService* hotReloadService);

    std::string GetName() const override { return "AssetRuntimeEditAdapter"; }
    bool CanHandle(const RuntimeEditCommand& command) const override;
    RuntimeEditResult Apply(const RuntimeEditCommand& command) override;
    std::vector<std::string> GetSupportedCommands() const override;

    void RegisterHandler(std::string commandName, ReloadHandler handler);
    void ClearHandlers();

    void SetHotReloadService(AssetHotReloadService* hotReloadService);
    void SetRuntimeResourceRegistry(RuntimeResourceRegistry* registry);
    RuntimeResourceRegistry* GetRuntimeResourceRegistry() { return _resourceRegistry; }
    const RuntimeResourceRegistry* GetRuntimeResourceRegistry() const { return _resourceRegistry; }
    AssetReloadPipeline& GetReloadPipeline() { return _reloadPipeline; }
    const AssetReloadPipeline& GetReloadPipeline() const { return _reloadPipeline; }

private:
    RuntimeEditResult ApplyDefault(const RuntimeEditCommand& command) const;
    RuntimeEditResult HandleWatch(const RuntimeEditCommand& command);
    RuntimeEditResult HandleUnwatch(const RuntimeEditCommand& command);
    RuntimeEditResult HandleReloadAll(const RuntimeEditCommand& command) const;
    RuntimeEditResult HandlePipelineCommand(const RuntimeEditCommand& command) const;
    AssetReloadRequest BuildReloadRequest(const RuntimeEditCommand& command) const;

    AssetHotReloadService* _hotReloadService = nullptr;
    RuntimeResourceRegistry* _resourceRegistry = nullptr;
    AssetReloadPipeline _reloadPipeline;
    std::unordered_map<std::string, ReloadHandler> _handlers;
};

} // namespace subspace
