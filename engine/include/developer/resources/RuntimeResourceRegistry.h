#pragma once

#include "developer/assets/AssetReloadPipeline.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct RuntimeResourceBinding {
    std::string kind;
    std::string systemName;
    std::string description;
    bool developmentOnly = true;
};

struct RuntimeResourceCommitReport {
    bool handled = false;
    bool success = false;
    bool committed = false;
    std::string kind;
    std::string assetId;
    std::string path;
    std::string systemName;
    std::string message;
    std::vector<std::string> warnings;
};

class RuntimeResourceRegistry {
public:
    using CommitCallback = std::function<RuntimeResourceCommitReport(const AssetReloadRequest&, const AssetReloadReport&)>;

    void RegisterBinding(RuntimeResourceBinding binding, CommitCallback callback);
    void UnregisterBinding(const std::string& kind);
    bool HasBinding(const std::string& kind) const;

    RuntimeResourceCommitReport Commit(const AssetReloadRequest& request, const AssetReloadReport& staged) const;
    std::vector<RuntimeResourceBinding> GetBindings() const;
    std::vector<std::string> GetSupportedKinds() const;
    void Clear();

private:
    struct Entry {
        RuntimeResourceBinding binding;
        CommitCallback callback;
    };

    std::unordered_map<std::string, Entry> _entries;
};

} // namespace subspace
