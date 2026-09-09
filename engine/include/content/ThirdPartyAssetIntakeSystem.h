#pragma once

#include "developer/provenance/AssetProvenanceManifest.h"

#include <string>
#include <vector>

namespace subspace {

struct ThirdPartyAssetDescriptor {
    std::string assetId;
    std::string title;
    std::string author;
    std::string sourceUrl;
    std::string licenseId;
    std::string licenseUrl;
    std::string attributionText;
    std::string checksumSha256;
    std::string upstreamVersion;
    std::string sourcePath;
    bool sourceReadOnly = true;
};

struct ThirdPartyAssetIntakeResult {
    bool accepted = false;
    std::string canonicalSourcePath;
    std::string derivedPath;
    std::vector<std::string> errors;
};

/// Pass318 governed third-party intake. External source payloads remain
/// provenance-backed and read-only; Subspace-authored derivatives live in a
/// separate path so replacing upstream art never changes gameplay identity.
class ThirdPartyAssetIntakeSystem {
public:
    ThirdPartyAssetIntakeResult ValidateAndRegister(
        const ThirdPartyAssetDescriptor& descriptor,
        AssetProvenanceManifest& manifest) const;

    static std::string SanitizeAssetId(const std::string& assetId);
};

} // namespace subspace
