#pragma once

#include "assets/CanonicalAsset.h"
#include "assets/CanonicalAssetRegistry.h"
#include "content/ShipyardModuleSystem.h"

#include <vector>

namespace subspace {

/// Projects the existing certified Shipyard module catalog into the canonical
/// asset schema without replacing the proven OBJ renderer. This is the first
/// runtime convergence point for Universal Kitbash identity.
class ShipyardCanonicalAssetBridge {
public:
    static assets::CanonicalAsset BuildAsset(const ShipyardModuleRecord& record);
    static std::size_t PopulateRegistry(assets::CanonicalAssetRegistry& registry,
                                        const std::vector<ShipyardModuleRecord>& catalog);
};

} // namespace subspace
