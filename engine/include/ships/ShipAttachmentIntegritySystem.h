#pragma once

#include "core/Math.h"
#include <string>
#include <vector>

namespace subspace {

enum class ShipMountKind { DirectHull, Adapter, Pylon, Fairing, Spine, Invalid };

struct ShipAttachmentProbe {
    std::string moduleId;
    ShipMountKind mount = ShipMountKind::Invalid;
    Vector3 moduleAnchor{};
    Vector3 hullAnchor{};
    float moduleRadius = 1.0f;
    float visibleGap = 0.0f;
    bool exhaustClear = true;
};

struct ShipAttachmentAudit {
    bool valid = true;
    int directMounts = 0;
    int bridgeMounts = 0;
    int rejected = 0;
    float largestGap = 0.0f;
    std::vector<std::string> errors;
};

class ShipAttachmentIntegritySystem {
public:
    static bool IsSupportedMount(ShipMountKind kind);
    static float SuggestedBridgeLength(const ShipAttachmentProbe& probe);
    ShipAttachmentAudit Audit(const std::vector<ShipAttachmentProbe>& probes) const;
};

} // namespace subspace
