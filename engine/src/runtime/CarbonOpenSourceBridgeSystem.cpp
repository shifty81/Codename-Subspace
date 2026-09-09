#include "runtime/CarbonOpenSourceBridgeSystem.h"

namespace subspace {
CarbonOpenSourceBridgeReport CarbonOpenSourceBridgeSystem::Build2026Plan() const {
    CarbonOpenSourceBridgeReport r;
    auto add=[&](const char*n,const char*u,CarbonBridgeDisposition d,const char*a,const char*why){r.components.push_back({n,u,"MIT",d,a,why});};
    add("core","https://github.com/carbonengine/core",CarbonBridgeDisposition::IsolatedLibraryCandidate,"platform/core","Generic low-level helpers may be reused only behind native adapters after dependency audit.");
    add("trinity","https://github.com/carbonengine/trinity",CarbonBridgeDisposition::ArchitectureReference,"rendering","Study large-scene rendering, materials and resource lifetimes without replacing the current native renderer in-place.");
    add("destiny","https://github.com/carbonengine/destiny",CarbonBridgeDisposition::NativeAdaptation,"physics/fleet/orbital simulation","Adapt interest-management and large-world simulation concepts to Subspace data; public repository still has external build dependencies.");
    add("io","https://github.com/carbonengine/io",CarbonBridgeDisposition::IsolatedLibraryCandidate,"networking","Evaluate low-level networking behind the future co-op transport interface.");
    add("pathfinder","https://github.com/carbonengine/pathfinder",CarbonBridgeDisposition::NativeAdaptation,"navigation/galaxy routing","Use generic graph-routing concepts with Subspace galaxy data rather than EVE map data.");
    add("localization","https://github.com/carbonengine/localization",CarbonBridgeDisposition::IsolatedLibraryCandidate,"ui/localization","Candidate for a future localization adapter after build and dependency verification.");
    add("audio","https://github.com/carbonengine/audio",CarbonBridgeDisposition::ArchitectureReference,"audio","Sound prioritization and spatial-audio concepts are useful; Wwise remains an external licensing/dependency decision.");
    add("scheduler","https://github.com/carbonengine/scheduler",CarbonBridgeDisposition::RejectedDirectRuntime,"runtime/jobs","Do not replace the native runtime with Carbon's Python/Greenlet scheduling assumptions.");
    add("blue","https://github.com/carbonengine/blue",CarbonBridgeDisposition::RejectedDirectRuntime,"runtime","Do not reintroduce a Python/C++ gameplay bridge after the native C++ conversion.");
    return r;
}
const CarbonOpenSourceComponent* CarbonOpenSourceBridgeSystem::Find(const CarbonOpenSourceBridgeReport&r,const std::string&n) const {for(const auto&c:r.components)if(c.name==n)return &c;return nullptr;}
} // namespace subspace
