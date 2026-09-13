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
        {"GameData/", ContentAuthority::ActiveContent, "canonical authored gameplay/runtime JSON and packaged runtime data"},
        {"content/", ContentAuthority::ActiveContent, "canonical governed metadata, schemas, registries, provenance, and derived authority"},
        {"content/generated/", ContentAuthority::GeneratedContent, "generated/certified derived content where explicitly governed"},
        {"Assets/", ContentAuthority::LegacyPendingAudit, "legacy uppercase asset root only when physically present"},
        {"assets/", ContentAuthority::LegacyPendingAudit, "legacy lowercase asset root only when physically present"},
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
