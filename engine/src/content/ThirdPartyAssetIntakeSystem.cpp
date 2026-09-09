#include "content/ThirdPartyAssetIntakeSystem.h"

#include <algorithm>
#include <cctype>

namespace subspace {

std::string ThirdPartyAssetIntakeSystem::SanitizeAssetId(const std::string& assetId)
{
    std::string out;
    out.reserve(assetId.size());
    for (char c : assetId) {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (std::isalnum(uc)) out.push_back(static_cast<char>(std::tolower(uc)));
        else if (c == '-' || c == '_' || c == ' ') out.push_back('_');
    }
    while (!out.empty() && out.front() == '_') out.erase(out.begin());
    while (!out.empty() && out.back() == '_') out.pop_back();
    return out;
}

ThirdPartyAssetIntakeResult ThirdPartyAssetIntakeSystem::ValidateAndRegister(
    const ThirdPartyAssetDescriptor& descriptor,
    AssetProvenanceManifest& manifest) const
{
    ThirdPartyAssetIntakeResult out;
    const std::string safeId = SanitizeAssetId(descriptor.assetId);
    if (safeId.empty()) out.errors.emplace_back("assetId is required");
    if (descriptor.title.empty()) out.errors.emplace_back("title is required");
    if (descriptor.author.empty()) out.errors.emplace_back("author is required");
    if (descriptor.sourceUrl.empty()) out.errors.emplace_back("sourceUrl is required");
    if (descriptor.licenseId.empty()) out.errors.emplace_back("licenseId is required");
    if (descriptor.licenseUrl.empty()) out.errors.emplace_back("licenseUrl is required");
    if (descriptor.attributionText.empty()) out.errors.emplace_back("attributionText is required");
    if (descriptor.checksumSha256.size() != 64) out.errors.emplace_back("checksumSha256 must contain 64 hex characters");
    else {
        const bool validHex = std::all_of(descriptor.checksumSha256.begin(), descriptor.checksumSha256.end(), [](char c) {
            return std::isxdigit(static_cast<unsigned char>(c)) != 0;
        });
        if (!validHex) out.errors.emplace_back("checksumSha256 contains non-hex characters");
    }
    if (!descriptor.sourceReadOnly) out.errors.emplace_back("third-party source payload must remain read-only");

    if (!out.errors.empty()) return out;

    out.canonicalSourcePath = "content/third_party/" + safeId + "/source/";
    out.derivedPath = "content/derived/" + safeId + "/";

    AssetProvenanceRecord record;
    record.assetId = safeId;
    record.originalTitle = descriptor.title;
    record.author = descriptor.author;
    record.sourceUrl = descriptor.sourceUrl;
    record.license = descriptor.licenseId;
    record.licenseUrl = descriptor.licenseUrl;
    record.attributionText = descriptor.attributionText;
    record.checksumSha256 = descriptor.checksumSha256;
    record.upstreamVersion = descriptor.upstreamVersion;
    record.sourcePath = descriptor.sourcePath;
    record.importedPath = out.canonicalSourcePath;
    record.derivedPath = out.derivedPath;
    record.modificationStatus = "UNMODIFIED_SOURCE";
    record.notes = "Governed third-party intake; source is read-only and derivatives are project-owned.";
    manifest.Upsert(std::move(record));
    out.accepted = true;
    return out;
}

} // namespace subspace
