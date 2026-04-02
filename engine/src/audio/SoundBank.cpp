#include "audio/SoundBank.h"

namespace subspace {

void SoundBank::Register(SoundAsset asset) {
    auto it = _byId.find(asset.id);
    if (it != _byId.end()) {
        // Replace existing asset
        std::size_t idx = it->second;
        _byName.erase(_assets[idx].name);
        _assets[idx] = asset;
        _byName[asset.name] = idx;
    } else {
        std::size_t idx = _assets.size();
        _byId[asset.id]     = idx;
        _byName[asset.name] = idx;
        _assets.push_back(std::move(asset));
    }
}

const SoundAsset* SoundBank::Get(SoundId id) const noexcept {
    auto it = _byId.find(id);
    return it != _byId.end() ? &_assets[it->second] : nullptr;
}

const SoundAsset* SoundBank::GetByName(const std::string& name) const noexcept {
    auto it = _byName.find(name);
    return it != _byName.end() ? &_assets[it->second] : nullptr;
}

void SoundBank::Clear() noexcept {
    _assets.clear();
    _byId.clear();
    _byName.clear();
}

} // namespace subspace
