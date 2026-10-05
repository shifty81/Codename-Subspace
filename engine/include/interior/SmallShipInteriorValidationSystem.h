#pragma once

#include "interior/ShipInteriorLayoutSystem.h"
#include "world/WorldScaleAuthoritySystem.h"

#include <algorithm>
#include <string>
#include <vector>

namespace subspace {

enum class SmallShipInteriorTier { Cutter, Shuttle, Corvette };

struct SmallShipInteriorProfile {
    SmallShipInteriorTier tier = SmallShipInteriorTier::Cutter;
    const char* label = "CUTTER";
    int maximumDecks = 1;
    std::size_t minimumWalkableVolumes = 1;
    std::size_t maximumWalkableVolumes = 2;
    bool requiresCockpit = true;
    bool requiresAirlockVolume = false;
};

struct SmallShipInteriorCertification {
    bool certified = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Deliberately certifies only small-ship interiors.  Larger/multi-deck hulls
/// must not become the debugging surface until these tiers are dependable with
/// the canonical player, pressure shell, portals and traversal.
class SmallShipInteriorValidationSystem {
public:
    static SmallShipInteriorProfile Profile(SmallShipInteriorTier tier) {
        switch (tier) {
            case SmallShipInteriorTier::Cutter: return {tier,"CUTTER",1,1,2,true,false};
            case SmallShipInteriorTier::Shuttle: return {tier,"SHUTTLE",1,2,4,true,true};
            case SmallShipInteriorTier::Corvette: return {tier,"CORVETTE",1,3,8,true,true};
        }
        return {};
    }

    static SmallShipInteriorCertification Validate(const InteriorLayoutPlan& layout,
                                                    SmallShipInteriorTier tier,
                                                    const WorldScaleProfile& scale) {
        SmallShipInteriorCertification out;
        const auto profile = Profile(tier);
        if (!layout.carve.valid) out.errors.push_back("Interior carve is not valid");
        if (!layout.shell.ready || layout.shell.surfaces.empty()) out.errors.push_back("Derived interior shell is not runtime-ready");
        if (layout.decks < 1 || layout.decks > profile.maximumDecks) out.errors.push_back("Small-ship tier exceeds certified deck count");
        if (layout.carve.walkableModuleCount < profile.minimumWalkableVolumes ||
            layout.carve.walkableModuleCount > profile.maximumWalkableVolumes)
            out.errors.push_back("Walkable cavity count is outside this small-ship tier");
        if (layout.carve.connectedWalkableCount != layout.carve.walkableModuleCount)
            out.errors.push_back("Every walkable cavity must connect to the command/root interior");

        bool cockpit = false, airlock = false;
        const float minimumHeadroom = scale.referencePlayerHeightMeters * 1.10f;
        const float minimumClearWidth = std::max(scale.referenceShoulderWidthMeters * 2.20f,
                                                 scale.referenceDoorWidthMeters * 0.90f);
        for (const auto& volume : layout.carve.volumes) {
            if (!volume.walkable) continue;
            cockpit = cockpit || volume.capability == ExteriorInteriorCapability::Cockpit ||
                                  volume.capability == ExteriorInteriorCapability::Bridge ||
                                  volume.roomType == InteriorRoomType::Cockpit;
            airlock = airlock || volume.capability == ExteriorInteriorCapability::Airlock ||
                                  volume.roomType == InteriorRoomType::Airlock;
            const float height = volume.halfExtents.z * 2.0f;
            const float clearWidth = std::min(volume.halfExtents.x, volume.halfExtents.y) * 2.0f;
            if (height + 0.001f < minimumHeadroom)
                out.errors.push_back("Walkable cavity does not clear the certified player height");
            if (clearWidth + 0.001f < minimumClearWidth)
                out.errors.push_back("Walkable cavity does not clear the certified player width/door envelope");
        }
        if (profile.requiresCockpit && !cockpit) out.errors.push_back("Small ship requires a physical cockpit/bridge cavity");
        if (profile.requiresAirlockVolume && !airlock) out.errors.push_back("This small-ship tier requires a physical airlock cavity");
        if (layout.carve.exclusions.empty())
            out.warnings.push_back("No machinery/weapon/surface exclusions were recorded; verify hull semantics before promotion");
        out.certified = out.errors.empty();
        return out;
    }
};

} // namespace subspace
