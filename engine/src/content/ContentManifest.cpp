#include "content/ContentManifest.h"

#include <sstream>

namespace subspace {

const char* ContentAuthorityName(ContentAuthority authority) {
    switch (authority) {
        case ContentAuthority::ActiveContent: return "ACTIVE_CONTENT";
        case ContentAuthority::GeneratedContent: return "GENERATED_CONTENT";
        case ContentAuthority::LegacyPendingAudit: return "LEGACY_PENDING_AUDIT";
        case ContentAuthority::ThirdPartyReference: return "THIRD_PARTY_REFERENCE";
        case ContentAuthority::MigrationSource: return "MIGRATION_SOURCE";
        default: return "UNKNOWN";
    }
}

std::vector<ContentManifestEntry> BuildDefaultContentManifestSeed() {
    return {
        {"content/assets/", ContentAuthority::ActiveContent, "canonical active assets after normalization"},
        {"content/data/", ContentAuthority::ActiveContent, "canonical active gameplay data"},
        {"content/generated/", ContentAuthority::GeneratedContent, "generated celestial/ship/station previews"},
        {"Assets/", ContentAuthority::LegacyPendingAudit, "legacy uppercase asset tree awaiting references audit"},
        {"assets/", ContentAuthority::LegacyPendingAudit, "legacy lowercase asset tree awaiting canonical merge"},
        {"GameData/", ContentAuthority::LegacyPendingAudit, "legacy gameplay data awaiting path normalization"},
        {"reference/third_party/pixel_planets/", ContentAuthority::ThirdPartyReference, "MIT visual reference to port"},
        {"reference/csharp-to-cpp-source/", ContentAuthority::MigrationSource, "C# behavior source-to-port backlog"}
    };
}

std::string ContentManifestSummary(const std::vector<ContentManifestEntry>& entries) {
    int active = 0, legacy = 0, reference = 0;
    for (const auto& entry : entries) {
        if (entry.authority == ContentAuthority::ActiveContent || entry.authority == ContentAuthority::GeneratedContent) { ++active; }
        if (entry.authority == ContentAuthority::LegacyPendingAudit) { ++legacy; }
        if (entry.authority == ContentAuthority::ThirdPartyReference || entry.authority == ContentAuthority::MigrationSource) { ++reference; }
    }
    std::ostringstream ss;
    ss << "content entries=" << entries.size() << " active=" << active << " legacy=" << legacy << " reference=" << reference;
    return ss.str();
}

} // namespace subspace
