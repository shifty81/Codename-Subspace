#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ContentAuthorityRole { ActiveRuntime, Generated, ReferenceOnly, MigrationSource, Quarantine };

struct ContentAuthorityEntry {
    std::string pathPrefix;
    ContentAuthorityRole role = ContentAuthorityRole::ReferenceOnly;
    std::string owner;
};

const char* ContentAuthorityRoleName(ContentAuthorityRole role);
std::vector<ContentAuthorityEntry> CreateDefaultContentAuthorityRegistry();
ContentAuthorityRole ResolveContentAuthorityRole(const std::vector<ContentAuthorityEntry>& registry, const std::string& path);

} // namespace subspace
