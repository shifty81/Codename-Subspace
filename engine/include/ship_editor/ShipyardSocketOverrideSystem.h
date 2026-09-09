#pragma once

#include "content/ShipyardModuleSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

/// Persistent manual-authoring socket overrides.
///
/// The generated/certified catalog remains the immutable source baseline. This
/// document stores only modules whose socket definitions were explicitly
/// changed in the in-game Shipyard editor. Loading replaces that module's
/// inferred socket vector with the reviewed/manual version.
class ShipyardSocketOverrideSystem {
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

    static bool SocketListsEqual(const std::vector<ShipyardAssemblySocket>& a,
                                 const std::vector<ShipyardAssemblySocket>& b,
                                 float epsilon = 1.0e-5f);
};

} // namespace subspace
