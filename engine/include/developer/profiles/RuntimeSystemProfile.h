#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class RuntimeSystemProfileKind {
    Minimal,
    Gameplay,
    DevelopmentPlay,
    Authoring,
    DedicatedServer,
    Tests,
};

struct RuntimeSystemProfile {
    RuntimeSystemProfileKind kind = RuntimeSystemProfileKind::Gameplay;
    std::string name;
    std::vector<std::string> enabledSystems;
    std::vector<std::string> disabledSystems;
};

class RuntimeSystemProfileCatalog {
public:
    RuntimeSystemProfileCatalog();
    void RegisterProfile(RuntimeSystemProfile profile);
    const RuntimeSystemProfile* Find(const std::string& name) const;
    RuntimeSystemProfile GetOrDefault(const std::string& name, RuntimeSystemProfile fallback = {}) const;
    std::vector<RuntimeSystemProfile> GetProfiles() const;

private:
    std::vector<RuntimeSystemProfile> _profiles;
};

const char* ToString(RuntimeSystemProfileKind kind);

} // namespace subspace
