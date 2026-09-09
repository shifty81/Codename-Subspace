#pragma once

#include "developer/resources/RuntimeResourceRegistry.h"

#include <string>
#include <vector>

namespace subspace {

struct RuntimeResourceBindingCatalogOptions {
    bool dryRunCommit = true;
    std::string systemNamePrefix = "DeveloperResource";
};

class RuntimeResourceBindingCatalog {
public:
    static std::vector<RuntimeResourceBinding> BuildDefaultBindings(const RuntimeResourceBindingCatalogOptions& options = {});
    static void RegisterDefaultBindings(RuntimeResourceRegistry& registry, const RuntimeResourceBindingCatalogOptions& options = {});
};

} // namespace subspace
