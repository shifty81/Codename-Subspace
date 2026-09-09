#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct WatchedAsset {
    std::string assetId;
    std::filesystem::path path;
    std::string kind;
    std::filesystem::file_time_type lastWriteTime{};
    bool exists = false;
};

struct AssetHotReloadChange {
    std::string assetId;
    std::filesystem::path path;
    std::string kind;
    bool appeared = false;
    bool disappeared = false;
    bool modified = false;
};

class AssetHotReloadService {
public:
    bool Watch(std::string assetId, std::filesystem::path path, std::string kind = {});
    bool Unwatch(const std::string& assetId);
    void Clear();

    std::vector<AssetHotReloadChange> PollChanges();
    std::vector<WatchedAsset> GetWatchedAssets() const;
    bool IsWatching(const std::string& assetId) const;

private:
    static bool TryGetWriteTime(const std::filesystem::path& path,
                                std::filesystem::file_time_type& outTime);

    std::unordered_map<std::string, WatchedAsset> _watched;
};

} // namespace subspace
