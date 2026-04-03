#pragma once

#include <any>
#include <cstdint>
#include <filesystem>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace subspace {

/// Opaque handle that uniquely identifies a loaded asset.
using AssetHandle = uint64_t;

/// Sentinel — invalid / null asset.
inline constexpr AssetHandle NullAsset = 0ull;

/// Central registry for loading, caching, and unloading typed engine assets.
///
/// Assets are identified by a monotonically increasing AssetHandle.
/// The manager maintains a reference count per handle.  GarbageCollect()
/// removes assets whose reference count has dropped to zero.
///
/// Template usage:
/// @code
///   AssetManager assets;
///
///   // Register a dummy texture asset
///   struct Texture { int width = 0, height = 0; };
///   AssetHandle h = assets.Load<Texture>("textures/ship.png");
///
///   Texture* tex = assets.Get<Texture>(h);
///   // ... use tex ...
///
///   assets.Unload(h);
///   assets.GarbageCollect();
/// @endcode
class AssetManager {
public:
    /// Load (or retrieve a cached copy of) an asset at the given path.
    ///
    /// If the path was already loaded the existing handle is returned and its
    /// reference count incremented.  The asset is default-constructed; a real
    /// backend would deserialise actual data here.
    ///
    /// @tparam T    Asset type (must be default-constructible).
    /// @param  path Filesystem path to the asset source file.
    /// @return Handle for use with Get<T>() and Unload().
    template<typename T>
    AssetHandle Load(const std::filesystem::path& path) {
        const std::string key = path.string();

        auto it = _pathToHandle.find(key);
        if (it != _pathToHandle.end()) {
            ++_refCounts[it->second];
            return it->second;
        }

        const AssetHandle handle = ++_nextHandle;
        _assets[handle]        = T{};
        _refCounts[handle]     = 1u;
        _pathToHandle[key]     = handle;
        _handleToPath[handle]  = key;
        _typeIndex.emplace(handle, std::type_index(typeid(T)));
        return handle;
    }

    /// Retrieve a pointer to the cached asset.
    /// @tparam T  Expected asset type.
    /// @return Pointer, or nullptr if the handle is invalid or type mismatches.
    template<typename T>
    T* Get(AssetHandle handle) {
        auto it = _assets.find(handle);
        if (it == _assets.end()) return nullptr;
        return std::any_cast<T>(&it->second);
    }

    /// @copydoc Get
    template<typename T>
    const T* Get(AssetHandle handle) const {
        auto it = _assets.find(handle);
        if (it == _assets.end()) return nullptr;
        return std::any_cast<T>(&it->second);
    }

    /// Decrement the reference count for an asset.
    /// The asset is not immediately removed; call GarbageCollect() to free it.
    void Unload(AssetHandle handle);

    /// Remove all assets whose reference count has reached zero.
    void GarbageCollect();

    /// Look up the path for a handle.  Returns empty string if unknown.
    std::string GetPath(AssetHandle handle) const;

    /// Current reference count for a handle (0 if unknown).
    uint32_t GetRefCount(AssetHandle handle) const;

    /// Total number of currently loaded assets.
    std::size_t LoadedCount() const noexcept { return _assets.size(); }

private:
    AssetHandle _nextHandle = 0ull;

    std::unordered_map<AssetHandle,  std::any>           _assets;
    std::unordered_map<AssetHandle,  uint32_t>           _refCounts;
    std::unordered_map<std::string,  AssetHandle>        _pathToHandle;
    std::unordered_map<AssetHandle,  std::string>        _handleToPath;
    std::unordered_map<AssetHandle,  std::type_index>    _typeIndex;
};

} // namespace subspace
