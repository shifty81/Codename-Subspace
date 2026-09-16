#include "editor/EditorAssetWorkbenchSystem.h"

namespace subspace {

const std::vector<EditorAssetDomainDescriptor>& EditorAssetWorkbenchSystem::Domains() {
    static const std::vector<EditorAssetDomainDescriptor> domains = {
        {EditorAssetDomain::ShipModules,    "SHIP MODULES",    "Shipyard",   true, true},
        {EditorAssetDomain::StationModules, "STATION MODULES", "Station",    true, false},
        {EditorAssetDomain::Interiors,      "INTERIORS",       "Interior",   true, false},
        {EditorAssetDomain::Characters,     "CHARACTERS",      "Character",  true, false},
        {EditorAssetDomain::Props,          "PROPS",           "Model",      true, false},
        {EditorAssetDomain::Materials,      "MATERIALS",       "Shading",    true, false},
        {EditorAssetDomain::Celestial,      "CELESTIAL",       "World",      true, false},
        {EditorAssetDomain::Vfx,            "VFX",             "Effects",    true, false},
        {EditorAssetDomain::Ui,             "UI",              "Interface",  true, false},
    };
    return domains;
}

const EditorAssetDomainDescriptor& EditorAssetWorkbenchSystem::Describe(EditorAssetDomain domain) {
    for (const auto& entry : Domains()) if (entry.domain == domain) return entry;
    return Domains().front();
}

bool EditorAssetWorkbenchSystem::HasLiveAdapter(EditorAssetDomain domain) {
    return Describe(domain).liveAdapter;
}

} // namespace subspace
