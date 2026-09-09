#pragma once

#include "content/ShipyardModuleSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

/// Persistent non-destructive Teach-PCG definition overrides.
/// Only reusable classification/generation metadata differences are stored.
class ShipyardDefinitionOverrideSystem {
public:
    static bool Save(const std::vector<ShipyardModuleRecord>& baseline,
                     const std::vector<ShipyardModuleRecord>& edited,
                     const std::string& path,
                     std::string* error = nullptr,
                     std::size_t* changedModules = nullptr);

    static bool LoadAndApply(std::vector<ShipyardModuleRecord>& catalog,
                             const std::string& path,
                             std::string* error = nullptr,
                             std::size_t* appliedModules = nullptr);

    static bool DefinitionEqual(const ShipyardModuleRecord& a,
                                const ShipyardModuleRecord& b,
                                float epsilon = 1.0e-5f);
};

} // namespace subspace
