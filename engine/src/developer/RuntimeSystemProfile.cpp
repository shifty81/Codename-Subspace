#include "developer/profiles/RuntimeSystemProfile.h"

#include <algorithm>

namespace subspace {

RuntimeSystemProfileCatalog::RuntimeSystemProfileCatalog()
{
    RegisterProfile({RuntimeSystemProfileKind::Minimal, "minimal", {"events", "ecs"}, {"rendering", "audio"}});
    RegisterProfile({RuntimeSystemProfileKind::Gameplay, "gameplay", {"events", "ecs", "physics", "combat", "navigation", "ui", "rendering"}, {}});
    RegisterProfile({RuntimeSystemProfileKind::DevelopmentPlay, "development_play", {"events", "ecs", "physics", "combat", "navigation", "ui", "rendering", "developer"}, {}});
    RegisterProfile({RuntimeSystemProfileKind::Authoring, "authoring", {"events", "ui", "developer", "assets"}, {"combat"}});
    RegisterProfile({RuntimeSystemProfileKind::DedicatedServer, "dedicated_server", {"events", "ecs", "physics", "combat", "navigation", "networking"}, {"rendering", "audio", "ui"}});
    RegisterProfile({RuntimeSystemProfileKind::Tests, "tests", {"events", "ecs"}, {"rendering", "audio", "networking"}});
}

void RuntimeSystemProfileCatalog::RegisterProfile(RuntimeSystemProfile profile)
{
    auto it = std::find_if(_profiles.begin(), _profiles.end(), [&](const auto& item) { return item.name == profile.name; });
    if (it == _profiles.end()) {
        _profiles.push_back(std::move(profile));
    } else {
        *it = std::move(profile);
    }
}

const RuntimeSystemProfile* RuntimeSystemProfileCatalog::Find(const std::string& name) const
{
    auto it = std::find_if(_profiles.begin(), _profiles.end(), [&](const auto& item) { return item.name == name; });
    return it == _profiles.end() ? nullptr : &*it;
}

RuntimeSystemProfile RuntimeSystemProfileCatalog::GetOrDefault(const std::string& name, RuntimeSystemProfile fallback) const
{
    if (const auto* profile = Find(name)) {
        return *profile;
    }
    return fallback;
}

std::vector<RuntimeSystemProfile> RuntimeSystemProfileCatalog::GetProfiles() const
{
    return _profiles;
}

const char* ToString(RuntimeSystemProfileKind kind)
{
    switch (kind) {
        case RuntimeSystemProfileKind::Minimal: return "minimal";
        case RuntimeSystemProfileKind::Gameplay: return "gameplay";
        case RuntimeSystemProfileKind::DevelopmentPlay: return "development_play";
        case RuntimeSystemProfileKind::Authoring: return "authoring";
        case RuntimeSystemProfileKind::DedicatedServer: return "dedicated_server";
        case RuntimeSystemProfileKind::Tests: return "tests";
    }
    return "unknown";
}

} // namespace subspace
