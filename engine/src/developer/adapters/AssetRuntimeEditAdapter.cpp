#include "developer/adapters/AssetRuntimeEditAdapter.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <utility>

namespace subspace {
namespace {
const std::vector<std::string>& Commands()
{
    static const std::vector<std::string> commands = {
        "asset.reload",
        "asset.reload.all",
        "asset.watch",
        "asset.unwatch",
        "asset.validate",
        "model.reload",
        "texture.reload",
        "material.reload",
        "shader.reload",
        "data.reload",
        "tuning.reload",
        "script.reload",
        "audio.reload",
        "ship.blueprint.reload"
    };
    return commands;
}

std::string FirstNonEmpty(std::initializer_list<std::string> values)
{
    for (const std::string& value : values) {
        if (!value.empty()) {
            return value;
        }
    }
    return {};
}

bool ParseBool(const std::string& value)
{
    return value == "1" || value == "true" || value == "yes" || value == "on";
}
}

AssetRuntimeEditAdapter::AssetRuntimeEditAdapter(AssetHotReloadService* hotReloadService)
    : _hotReloadService(hotReloadService)
{
}

bool AssetRuntimeEditAdapter::CanHandle(const RuntimeEditCommand& command) const
{
    const auto& commands = Commands();
    return std::find(commands.begin(), commands.end(), command.name) != commands.end();
}

RuntimeEditResult AssetRuntimeEditAdapter::Apply(const RuntimeEditCommand& command)
{
    const auto found = _handlers.find(command.name);
    if (found != _handlers.end()) {
        return found->second(command);
    }

    if (command.name == "asset.watch") {
        return HandleWatch(command);
    }
    if (command.name == "asset.unwatch") {
        return HandleUnwatch(command);
    }
    if (command.name == "asset.reload.all") {
        return HandleReloadAll(command);
    }
    if (command.name == "asset.reload" || command.name == "asset.validate" || command.name.find(".reload") != std::string::npos) {
        return HandlePipelineCommand(command);
    }

    return ApplyDefault(command);
}

std::vector<std::string> AssetRuntimeEditAdapter::GetSupportedCommands() const
{
    return Commands();
}

void AssetRuntimeEditAdapter::RegisterHandler(std::string commandName, ReloadHandler handler)
{
    if (!commandName.empty() && handler) {
        _handlers[std::move(commandName)] = std::move(handler);
    }
}

void AssetRuntimeEditAdapter::ClearHandlers()
{
    _handlers.clear();
}

void AssetRuntimeEditAdapter::SetHotReloadService(AssetHotReloadService* hotReloadService)
{
    _hotReloadService = hotReloadService;
}

void AssetRuntimeEditAdapter::SetRuntimeResourceRegistry(RuntimeResourceRegistry* registry)
{
    _resourceRegistry = registry;
}

RuntimeEditResult AssetRuntimeEditAdapter::HandleWatch(const RuntimeEditCommand& command)
{
    if (!_hotReloadService) {
        RuntimeEditResult result = RuntimeEditResult::Failure(command, "asset.watch requires an AssetHotReloadService binding.");
        result.warnings.push_back("Create AssetRuntimeEditAdapter with a hot-reload service or call SetHotReloadService().");
        return result;
    }

    const std::string path = command.GetArg("path");
    const std::string id = FirstNonEmpty({command.GetArg("id"), command.GetArg("asset"), path});
    std::string kind = command.GetArg("kind");
    if (kind.empty()) {
        kind = AssetReloadPipeline::InferKindFromPath(path);
    }

    if (path.empty() || id.empty()) {
        return RuntimeEditResult::Failure(command, "asset.watch requires path=... and either id=... or a usable path fallback.");
    }

    const bool watched = _hotReloadService->Watch(id, path, kind);
    RuntimeEditResult result = watched
        ? RuntimeEditResult::Success(command, "Watching asset '" + id + "' as kind '" + kind + "'.", false)
        : RuntimeEditResult::Failure(command, "Failed to watch asset '" + id + "'.");
    return result;
}

RuntimeEditResult AssetRuntimeEditAdapter::HandleUnwatch(const RuntimeEditCommand& command)
{
    if (!_hotReloadService) {
        RuntimeEditResult result = RuntimeEditResult::Failure(command, "asset.unwatch requires an AssetHotReloadService binding.");
        result.warnings.push_back("Create AssetRuntimeEditAdapter with a hot-reload service or call SetHotReloadService().");
        return result;
    }

    const std::string id = FirstNonEmpty({command.GetArg("id"), command.GetArg("asset"), command.GetArg("path")});
    if (id.empty()) {
        return RuntimeEditResult::Failure(command, "asset.unwatch requires id=... or asset=....");
    }

    const bool unwatched = _hotReloadService->Unwatch(id);
    return unwatched
        ? RuntimeEditResult::Success(command, "Stopped watching asset '" + id + "'.", false)
        : RuntimeEditResult::Failure(command, "Asset was not being watched: " + id);
}

RuntimeEditResult AssetRuntimeEditAdapter::HandleReloadAll(const RuntimeEditCommand& command) const
{
    if (!_hotReloadService) {
        RuntimeEditResult result = RuntimeEditResult::Failure(command, "asset.reload.all requires an AssetHotReloadService binding.");
        result.warnings.push_back("Only watched assets can be reloaded in bulk at this layer.");
        return result;
    }

    const std::vector<WatchedAsset> watched = _hotReloadService->GetWatchedAssets();
    if (watched.empty()) {
        return RuntimeEditResult::Success(command, "No watched assets to reload.", false);
    }

    std::size_t successCount = 0;
    std::vector<std::string> warnings;
    for (const WatchedAsset& asset : watched) {
        RuntimeEditCommand child = command;
        child.name = "asset.reload";
        child.SetArg("id", asset.assetId);
        child.SetArg("path", asset.path.string());
        child.SetArg("kind", asset.kind);
        AssetReloadReport report = _reloadPipeline.Reload(BuildReloadRequest(child));
        if (report.success) {
            ++successCount;
        }
        warnings.insert(warnings.end(), report.warnings.begin(), report.warnings.end());
        if (!report.success && !report.message.empty()) {
            warnings.push_back(report.message);
        }
    }

    std::ostringstream message;
    message << "Reloaded " << successCount << " of " << watched.size() << " watched assets.";
    RuntimeEditResult result = successCount == watched.size()
        ? RuntimeEditResult::Success(command, message.str(), false)
        : RuntimeEditResult::Failure(command, message.str());
    result.warnings = std::move(warnings);
    return result;
}

RuntimeEditResult AssetRuntimeEditAdapter::HandlePipelineCommand(const RuntimeEditCommand& command) const
{
    const AssetReloadRequest request = BuildReloadRequest(command);
    const bool validateOnly = command.name == "asset.validate" || request.validateOnly;
    AssetReloadReport report = validateOnly ? _reloadPipeline.Validate(request) : _reloadPipeline.Reload(request);

    if (!validateOnly && report.success && _resourceRegistry && _resourceRegistry->HasBinding(request.kind)) {
        RuntimeResourceCommitReport commit = _resourceRegistry->Commit(request, report);
        RuntimeEditResult result = commit.success
            ? RuntimeEditResult::Success(command, commit.message.empty() ? report.message : commit.message, false)
            : RuntimeEditResult::Failure(command, commit.message.empty() ? report.message : commit.message);
        result.warnings = report.warnings;
        result.warnings.insert(result.warnings.end(), commit.warnings.begin(), commit.warnings.end());
        return result;
    }

    return report.ToRuntimeEditResult(command);
}

AssetReloadRequest AssetRuntimeEditAdapter::BuildReloadRequest(const RuntimeEditCommand& command) const
{
    AssetReloadRequest request;
    request.commandName = command.name;
    request.assetId = FirstNonEmpty({command.GetArg("id"), command.GetArg("asset")});
    request.path = command.GetArg("path");
    request.kind = command.GetArg("kind", AssetReloadPipeline::InferKindFromCommand(command.name));
    if (request.kind.empty()) {
        request.kind = AssetReloadPipeline::InferKindFromPath(request.path);
    }
    request.force = ParseBool(command.GetArg("force"));
    request.validateOnly = command.name == "asset.validate" || ParseBool(command.GetArg("validateOnly"));
    request.sourceCommand = command;
    if (request.assetId.empty()) {
        request.assetId = request.path.string();
    }
    return request;
}

RuntimeEditResult AssetRuntimeEditAdapter::ApplyDefault(const RuntimeEditCommand& command) const
{
    RuntimeEditResult result = RuntimeEditResult::Success(command, {}, false);

    const std::string path = command.GetArg("path");
    const std::string id = command.GetArg("id", command.GetArg("asset", path));
    std::ostringstream message;

    if (command.name == "asset.reload.all") {
        message << "Asset reload-all command accepted; no concrete asset registry handler is registered yet.";
    } else if (command.name == "asset.watch") {
        message << "Asset watch command accepted for '" << (id.empty() ? path : id) << "'.";
    } else if (command.name == "asset.unwatch") {
        message << "Asset unwatch command accepted for '" << (id.empty() ? path : id) << "'.";
    } else if (path.empty() && id.empty()) {
        result = RuntimeEditResult::Failure(command, "Asset command requires path=... or id=... until a concrete registry handler is connected.");
        return result;
    } else {
        message << command.name << " accepted for '" << (id.empty() ? path : id) << "'.";
        if (!path.empty()) {
            std::error_code ec;
            if (!std::filesystem::exists(path, ec)) {
                result.warnings.push_back("Path does not exist yet; reload will need a concrete registry/staging handler.");
            }
        }
    }

    result.message = message.str();
    result.warnings.push_back("Default asset adapter is a safe bridge only; register concrete reload handlers for live renderer/content swaps.");
    return result;
}

} // namespace subspace
