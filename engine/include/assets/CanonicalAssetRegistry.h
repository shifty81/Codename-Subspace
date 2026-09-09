#pragma once

#include "assets/CanonicalAsset.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace::assets {

/// Runtime lookup authority for normalized/certified content identities.
/// Pass533A intentionally keeps this registry small and data-only: renderers,
/// Shipyard authoring and itemization can resolve the same stable asset id
/// without changing the existing OBJ render path yet.
class CanonicalAssetRegistry {
public:
    void Clear();
    bool Upsert(CanonicalAsset asset);
    bool Contains(const std::string& assetId) const;
    const CanonicalAsset* Find(const std::string& assetId) const;
    std::size_t Size() const { return assets_.size(); }
    std::vector<std::string> AssetIds() const;

private:
    std::unordered_map<std::string, CanonicalAsset> assets_;
};

} // namespace subspace::assets
