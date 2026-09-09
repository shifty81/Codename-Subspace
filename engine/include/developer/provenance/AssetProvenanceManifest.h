#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct AssetProvenanceRecord {
    std::string assetId;
    std::string originalTitle;
    std::string sourcePath;
    std::string importedPath;
    std::string derivedPath;
    std::string sourceUrl;
    std::string license;
    std::string licenseUrl;
    std::string author;
    std::string attributionText;
    std::string checksumSha256;
    std::string upstreamVersion;
    std::string modificationStatus;
    std::string notes;
};

class AssetProvenanceManifest {
public:
    void Upsert(AssetProvenanceRecord record);
    const AssetProvenanceRecord* Find(const std::string& assetId) const;
    std::vector<AssetProvenanceRecord> GetRecords() const;
    bool Empty() const { return _records.empty(); }
    void Clear();

private:
    std::unordered_map<std::string, AssetProvenanceRecord> _records;
};

} // namespace subspace
