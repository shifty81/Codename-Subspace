#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ContentAuthority {
    ActiveContent,
    GeneratedContent,
    LegacyPendingAudit,
    ThirdPartyReference,
    MigrationSource
};

struct ContentManifestEntry {
    std::string path;
    ContentAuthority authority = ContentAuthority::ActiveContent;
    std::string purpose;
};

const char* ContentAuthorityName(ContentAuthority authority);
std::vector<ContentManifestEntry> BuildDefaultContentManifestSeed();
std::string ContentManifestSummary(const std::vector<ContentManifestEntry>& entries);

} // namespace subspace
