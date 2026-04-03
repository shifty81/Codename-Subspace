#include "core/assets/AssetManager.h"

namespace subspace {

void AssetManager::Unload(AssetHandle handle) {
    auto it = _refCounts.find(handle);
    if (it == _refCounts.end()) return;
    if (it->second > 0) --it->second;
}

void AssetManager::GarbageCollect() {
    for (auto it = _refCounts.begin(); it != _refCounts.end(); ) {
        if (it->second == 0) {
            AssetHandle h = it->first;
            _assets.erase(h);
            auto pathIt = _handleToPath.find(h);
            if (pathIt != _handleToPath.end()) {
                _pathToHandle.erase(pathIt->second);
                _handleToPath.erase(pathIt);
            }
            _typeIndex.erase(h);
            it = _refCounts.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AssetManager::GetPath(AssetHandle handle) const {
    auto it = _handleToPath.find(handle);
    return it != _handleToPath.end() ? it->second : std::string{};
}

uint32_t AssetManager::GetRefCount(AssetHandle handle) const {
    auto it = _refCounts.find(handle);
    return it != _refCounts.end() ? it->second : 0u;
}

} // namespace subspace
