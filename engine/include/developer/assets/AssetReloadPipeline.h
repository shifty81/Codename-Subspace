#pragma once

#include "developer/RuntimeEditCommand.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class AssetReloadStage {
    Requested,
    Staged,
    Validated,
    Committed,
    Failed
};

struct AssetReloadRequest {
    std::string commandName;
    std::string assetId;
    std::filesystem::path path;
    std::string kind;
    bool force = false;
    bool validateOnly = false;
    RuntimeEditCommand sourceCommand;
};

struct AssetReloadReport {
    bool handled = false;
    bool success = false;
    bool staged = false;
    bool validated = false;
    bool committed = false;
    AssetReloadStage stage = AssetReloadStage::Requested;
    std::string assetId;
    std::filesystem::path path;
    std::string kind;
    std::uintmax_t byteSize = 0;
    std::string message;
    std::vector<std::string> warnings;

    RuntimeEditResult ToRuntimeEditResult(RuntimeEditCommand command) const;
};

struct AssetReloadHandler {
    std::string kind;
    std::vector<std::string> allowedExtensions;
    bool requiresExistingFile = true;
    bool commitWithoutCallbackSucceeds = true;

    using StageCallback = std::function<AssetReloadReport(const AssetReloadRequest&)>;
    using ValidateCallback = std::function<AssetReloadReport(const AssetReloadRequest&, const AssetReloadReport&)>;
    using CommitCallback = std::function<AssetReloadReport(const AssetReloadRequest&, const AssetReloadReport&)>;

    StageCallback stage;
    ValidateCallback validate;
    CommitCallback commit;
};

class AssetReloadPipeline {
public:
    AssetReloadPipeline();

    void RegisterHandler(AssetReloadHandler handler);
    void RegisterCommitCallback(const std::string& kind, AssetReloadHandler::CommitCallback callback);
    void ClearHandlers();
    void RegisterDefaultFileHandlers();

    AssetReloadReport Reload(const AssetReloadRequest& request) const;
    AssetReloadReport Validate(const AssetReloadRequest& request) const;

    bool HasHandler(const std::string& kind) const;
    std::vector<std::string> GetRegisteredKinds() const;

    static std::string InferKindFromPath(const std::filesystem::path& path);
    static std::string InferKindFromCommand(const std::string& commandName);
    static std::string NormalizeKind(std::string kind);

private:
    AssetReloadReport Run(const AssetReloadRequest& request, bool validateOnly) const;
    AssetReloadReport StageFileAsset(const AssetReloadRequest& request, const AssetReloadHandler& handler) const;
    AssetReloadReport ValidateFileAsset(const AssetReloadRequest& request,
                                        const AssetReloadHandler& handler,
                                        const AssetReloadReport& staged) const;
    AssetReloadReport CommitFileAsset(const AssetReloadRequest& request,
                                      const AssetReloadHandler& handler,
                                      const AssetReloadReport& validated) const;

    const AssetReloadHandler* FindHandler(const std::string& kind) const;

    std::unordered_map<std::string, AssetReloadHandler> _handlers;
};

} // namespace subspace
