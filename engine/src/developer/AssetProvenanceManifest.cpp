#include "developer/provenance/AssetProvenanceManifest.h"

#include <algorithm>

namespace subspace {

void AssetProvenanceManifest::Upsert(AssetProvenanceRecord record)
{
    if (!record.assetId.empty()) {
        _records[record.assetId] = std::move(record);
    }
}

const AssetProvenanceRecord* AssetProvenanceManifest::Find(const std::string& assetId) const
{
    auto it = _records.find(assetId);
    return it == _records.end() ? nullptr : &it->second;
}

std::vector<AssetProvenanceRecord> AssetProvenanceManifest::GetRecords() const
{
    std::vector<AssetProvenanceRecord> result;
    result.reserve(_records.size());
    for (const auto& kv : _records) {
        result.push_back(kv.second);
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.assetId < b.assetId; });
    return result;
}

void AssetProvenanceManifest::Clear()
{
    _records.clear();
}

} // namespace subspace
