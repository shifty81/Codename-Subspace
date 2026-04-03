#pragma once

#include "audio/SoundAsset.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

/// Registry that owns a collection of SoundAsset definitions.
///
/// Assets are keyed by both their numeric SoundId and a human-readable name
/// so callers can look them up either way.
class SoundBank {
public:
    /// Register a sound asset.  Replaces any existing asset with the same id.
    void Register(SoundAsset asset);

    /// Look up by id.  Returns nullptr if not found.
    const SoundAsset* Get(SoundId id)             const noexcept;

    /// Look up by name.  Returns nullptr if not found.
    const SoundAsset* GetByName(const std::string& name) const noexcept;

    /// Number of registered assets.
    std::size_t Count() const noexcept { return _byId.size(); }

    /// All registered assets (ordered by registration).
    const std::vector<SoundAsset>& Assets() const noexcept { return _assets; }

    /// Remove all registered assets.
    void Clear() noexcept;

private:
    std::vector<SoundAsset>                     _assets;
    std::unordered_map<SoundId,     std::size_t> _byId;
    std::unordered_map<std::string, std::size_t> _byName;
};

} // namespace subspace
