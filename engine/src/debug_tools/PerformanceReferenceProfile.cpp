#include "debug_tools/PerformanceReferenceProfile.h"

namespace subspace {
PerformanceReferenceProfile PerformanceReferenceProfiles::P921Reference() {
    PerformanceReferenceProfile p;
    p.baselineId = "SUBSPACE-P921-PERFORMANCE-REFERENCE-BASELINE";
    p.certifiedGateId = "QG-20260912-193645-full-58693c78";
    p.metrics = {
        {"client.startup", "Game client startup-to-interactive responsiveness", true},
        {"editor.startup", "Shipyard Editor startup-to-interactive responsiveness", true},
        {"runtime.frame_pacing", "Interactive game frame pacing and hitch distribution", true},
        {"editor.input_latency", "Viewport camera/selection/manipulation responsiveness", true},
        {"transition.hitch", "Docking/workspace/world-transition hitch duration", true},
        {"memory.long_session", "Working-set growth during repeated travel/editor cycles", true}
    };
    return p;
}
} // namespace subspace
