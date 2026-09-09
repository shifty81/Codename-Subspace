#include "developer/resources/RuntimeResourceRegistry.h"

#include <algorithm>

namespace subspace {

void RuntimeResourceRegistry::RegisterBinding(RuntimeResourceBinding binding, CommitCallback callback)
{
    if (binding.kind.empty()) {
        return;
    }
    const std::string key = binding.kind;
    _entries[key] = Entry{std::move(binding), std::move(callback)};
}

void RuntimeResourceRegistry::UnregisterBinding(const std::string& kind)
{
    _entries.erase(kind);
}

bool RuntimeResourceRegistry::HasBinding(const std::string& kind) const
{
    return _entries.find(kind) != _entries.end();
}

RuntimeResourceCommitReport RuntimeResourceRegistry::Commit(const AssetReloadRequest& request, const AssetReloadReport& staged) const
{
    RuntimeResourceCommitReport report;
    report.kind = request.kind;
    report.assetId = request.assetId;
    report.path = request.path.string();

    auto it = _entries.find(request.kind);
    if (it == _entries.end()) {
        report.message = "No runtime resource handler registered for kind: " + request.kind;
        return report;
    }

    report.handled = true;
    report.systemName = it->second.binding.systemName;

    if (!staged.success) {
        report.message = "Staged asset failed validation; runtime commit skipped.";
        report.warnings = staged.warnings;
        return report;
    }

    if (!it->second.callback) {
        report.message = "Runtime resource handler has no callback for kind: " + request.kind;
        return report;
    }

    report = it->second.callback(request, staged);
    report.handled = true;
    report.kind = request.kind;
    report.assetId = request.assetId;
    report.path = request.path.string();
    if (report.systemName.empty()) {
        report.systemName = it->second.binding.systemName;
    }
    return report;
}

std::vector<RuntimeResourceBinding> RuntimeResourceRegistry::GetBindings() const
{
    std::vector<RuntimeResourceBinding> bindings;
    bindings.reserve(_entries.size());
    for (const auto& kv : _entries) {
        bindings.push_back(kv.second.binding);
    }
    std::sort(bindings.begin(), bindings.end(), [](const auto& a, const auto& b) {
        return a.kind < b.kind;
    });
    return bindings;
}

std::vector<std::string> RuntimeResourceRegistry::GetSupportedKinds() const
{
    std::vector<std::string> kinds;
    kinds.reserve(_entries.size());
    for (const auto& kv : _entries) {
        kinds.push_back(kv.first);
    }
    std::sort(kinds.begin(), kinds.end());
    return kinds;
}

void RuntimeResourceRegistry::Clear()
{
    _entries.clear();
}

} // namespace subspace
