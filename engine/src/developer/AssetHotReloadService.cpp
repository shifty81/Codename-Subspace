#include "developer/AssetHotReloadService.h"

#include <utility>

namespace subspace {

bool AssetHotReloadService::Watch(std::string assetId, std::filesystem::path path, std::string kind)
{
    if (assetId.empty()) {
        assetId = path.string();
    }

    WatchedAsset watched;
    watched.assetId = assetId;
    watched.path = std::move(path);
    watched.kind = std::move(kind);
    watched.exists = TryGetWriteTime(watched.path, watched.lastWriteTime);

    _watched[watched.assetId] = std::move(watched);
    return true;
}

bool AssetHotReloadService::Unwatch(const std::string& assetId)
{
    return _watched.erase(assetId) > 0;
}

void AssetHotReloadService::Clear()
{
    _watched.clear();
}

std::vector<AssetHotReloadChange> AssetHotReloadService::PollChanges()
{
    std::vector<AssetHotReloadChange> changes;

    for (auto& [id, watched] : _watched) {
        std::filesystem::file_time_type currentWriteTime{};
        const bool existsNow = TryGetWriteTime(watched.path, currentWriteTime);

        AssetHotReloadChange change;
        change.assetId = watched.assetId;
        change.path = watched.path;
        change.kind = watched.kind;

        if (!watched.exists && existsNow) {
            change.appeared = true;
            change.modified = true;
        } else if (watched.exists && !existsNow) {
            change.disappeared = true;
        } else if (existsNow && currentWriteTime != watched.lastWriteTime) {
            change.modified = true;
        }

        if (change.appeared || change.disappeared || change.modified) {
            changes.push_back(change);
        }

        watched.exists = existsNow;
        watched.lastWriteTime = currentWriteTime;
    }

    return changes;
}

std::vector<WatchedAsset> AssetHotReloadService::GetWatchedAssets() const
{
    std::vector<WatchedAsset> watched;
    watched.reserve(_watched.size());
    for (const auto& [_, item] : _watched) {
        watched.push_back(item);
    }
    return watched;
}

bool AssetHotReloadService::IsWatching(const std::string& assetId) const
{
    return _watched.find(assetId) != _watched.end();
}

bool AssetHotReloadService::TryGetWriteTime(const std::filesystem::path& path,
                                            std::filesystem::file_time_type& outTime)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        return false;
    }

    outTime = std::filesystem::last_write_time(path, ec);
    return !ec;
}

} // namespace subspace
