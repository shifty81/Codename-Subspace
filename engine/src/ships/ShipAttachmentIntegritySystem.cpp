#include "ships/ShipAttachmentIntegritySystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {

bool ShipAttachmentIntegritySystem::IsSupportedMount(ShipMountKind kind) {
    return kind != ShipMountKind::Invalid;
}

float ShipAttachmentIntegritySystem::SuggestedBridgeLength(const ShipAttachmentProbe& probe) {
    const Vector3 delta = probe.moduleAnchor - probe.hullAnchor;
    return std::max(0.0f, delta.length() - std::max(0.0f, probe.moduleRadius * 0.55f));
}

ShipAttachmentAudit ShipAttachmentIntegritySystem::Audit(const std::vector<ShipAttachmentProbe>& probes) const {
    ShipAttachmentAudit out;
    for (const auto& p : probes) {
        const float bridge = SuggestedBridgeLength(p);
        out.largestGap = std::max(out.largestGap, std::max(p.visibleGap, bridge));
        const bool mounted = IsSupportedMount(p.mount);
        const bool gapCovered = p.mount == ShipMountKind::DirectHull ? p.visibleGap <= 0.18f : bridge <= std::max(0.35f, p.visibleGap + p.moduleRadius * 3.0f);
        if (!mounted || !gapCovered || !p.exhaustClear) {
            out.valid = false;
            ++out.rejected;
            if (!mounted) out.errors.push_back(p.moduleId + ": no physical mount");
            else if (!gapCovered) out.errors.push_back(p.moduleId + ": unsupported visual gap");
            else out.errors.push_back(p.moduleId + ": blocked exhaust clearance");
            continue;
        }
        if (p.mount == ShipMountKind::DirectHull) ++out.directMounts;
        else ++out.bridgeMounts;
    }
    return out;
}

} // namespace subspace
