#include "content/ContentAuthorityRegistry.h"

namespace subspace {

const char* ContentAuthorityRoleName(ContentAuthorityRole role) {
    switch (role) {
    case ContentAuthorityRole::ActiveRuntime: return "ActiveRuntime";
    case ContentAuthorityRole::Generated: return "Generated";
    case ContentAuthorityRole::ReferenceOnly: return "ReferenceOnly";
    case ContentAuthorityRole::MigrationSource: return "MigrationSource";
    case ContentAuthorityRole::Quarantine: return "Quarantine";
    }
    return "Unknown";
}

std::vector<ContentAuthorityEntry> CreateDefaultContentAuthorityRegistry() {
    return {
        {"GameData/", ContentAuthorityRole::ActiveRuntime, "runtime-authored-data"},
        {"content/", ContentAuthorityRole::ActiveRuntime, "governed-metadata"},
        {"content/generated/", ContentAuthorityRole::Generated, "pipeline"},
        {"reference/", ContentAuthorityRole::ReferenceOnly, "migration"},
        {"AvorionLike/", ContentAuthorityRole::MigrationSource, "cpp-conversion"},
        {"Assets/", ContentAuthorityRole::MigrationSource, "legacy-asset-root"},
        {"assets/", ContentAuthorityRole::MigrationSource, "legacy-asset-root"},
    };
}

ContentAuthorityRole ResolveContentAuthorityRole(const std::vector<ContentAuthorityEntry>& registry, const std::string& path) {
    ContentAuthorityRole result = ContentAuthorityRole::Quarantine;
    std::size_t best = 0;
    for (const auto& entry : registry) {
        if (path.rfind(entry.pathPrefix, 0) == 0 && entry.pathPrefix.size() >= best) {
            best = entry.pathPrefix.size();
            result = entry.role;
        }
    }
    return result;
}

} // namespace subspace
