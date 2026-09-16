#pragma once

#include <string>
#include <vector>

namespace subspace {

// PASS1444-1453: one normalized DCC shell hosts asset-authoring domains.
// Ship modules are the first live adapter. Additional domains deliberately
// advertise their migration state rather than pretending every editor is wired.
enum class EditorAssetDomain {
    ShipModules,
    StationModules,
    Interiors,
    Characters,
    Props,
    Materials,
    Celestial,
    Vfx,
    Ui
};

struct EditorAssetDomainDescriptor {
    EditorAssetDomain domain = EditorAssetDomain::ShipModules;
    std::string label;
    std::string workspaceHint;
    bool shellAvailable = true;
    bool liveAdapter = false;
};

class EditorAssetWorkbenchSystem {
public:
    static const std::vector<EditorAssetDomainDescriptor>& Domains();
    static const EditorAssetDomainDescriptor& Describe(EditorAssetDomain domain);
    static bool HasLiveAdapter(EditorAssetDomain domain);
};

} // namespace subspace
