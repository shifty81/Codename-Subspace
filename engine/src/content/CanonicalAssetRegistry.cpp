#include "assets/CanonicalAssetRegistry.h"

#include <algorithm>

namespace subspace::assets {

void CanonicalAssetRegistry::Clear() { assets_.clear(); }

bool CanonicalAssetRegistry::Upsert(CanonicalAsset asset) {
    if(asset.assetId.empty()) return false;
    assets_[asset.assetId]=std::move(asset);
    return true;
}

bool CanonicalAssetRegistry::Contains(const std::string& assetId) const {
    return assets_.find(assetId)!=assets_.end();
}

const CanonicalAsset* CanonicalAssetRegistry::Find(const std::string& assetId) const {
    const auto it=assets_.find(assetId);
    return it==assets_.end()?nullptr:&it->second;
}

std::vector<std::string> CanonicalAssetRegistry::AssetIds() const {
    std::vector<std::string> ids;ids.reserve(assets_.size());
    for(const auto& kv:assets_)ids.push_back(kv.first);
    std::sort(ids.begin(),ids.end());
    return ids;
}

} // namespace subspace::assets
