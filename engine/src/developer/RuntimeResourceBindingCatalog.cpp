#include "developer/resources/RuntimeResourceBindingCatalog.h"

namespace subspace {

std::vector<RuntimeResourceBinding> RuntimeResourceBindingCatalog::BuildDefaultBindings(const RuntimeResourceBindingCatalogOptions& options) {
    const std::vector<std::pair<std::string, std::string>> defaults = {
        {"texture", "Texture runtime reload boundary"},
        {"model", "Model/GLB runtime reload boundary"},
        {"material", "Material parameter/runtime reload boundary"},
        {"shader", "Shader compile-and-swap boundary"},
        {"data", "Generic data table reload boundary"},
        {"tuning", "Gameplay tuning reload boundary"},
        {"script", "Script or behavior data reload boundary"},
        {"audio", "Audio resource reload boundary"},
        {"ship.blueprint", "Ship blueprint reload boundary"},
        {"generic", "Generic developer asset reload boundary"}
    };

    std::vector<RuntimeResourceBinding> bindings;
    bindings.reserve(defaults.size());
    for (const auto& entry : defaults) {
        RuntimeResourceBinding binding;
        binding.kind = entry.first;
        binding.systemName = options.systemNamePrefix + ":" + entry.first;
        binding.description = entry.second;
        binding.developmentOnly = true;
        bindings.push_back(binding);
    }
    return bindings;
}

void RuntimeResourceBindingCatalog::RegisterDefaultBindings(RuntimeResourceRegistry& registry, const RuntimeResourceBindingCatalogOptions& options) {
    for (const auto& binding : BuildDefaultBindings(options)) {
        registry.RegisterBinding(binding, [binding, options](const AssetReloadRequest& request, const AssetReloadReport& staged) {
            RuntimeResourceCommitReport report;
            report.handled = true;
            report.success = staged.success;
            report.committed = staged.success && !options.dryRunCommit;
            report.kind = request.kind;
            report.assetId = request.assetId;
            report.path = request.path.string();
            report.systemName = binding.systemName;
            report.warnings = staged.warnings;
            if (!staged.success) {
                report.message = "Runtime resource staging failed: " + staged.message;
            } else if (options.dryRunCommit) {
                report.message = "Runtime resource staged successfully; commit callback is still dry-run.";
            } else {
                report.message = "Runtime resource staged and committed by default callback.";
            }
            return report;
        });
    }
}

} // namespace subspace
