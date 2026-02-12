#pragma once

#include "factions/SilhouetteProfile.h"

#include <string>
#include <vector>

namespace subspace {

enum class HullArchetype { Brick, Needle, Disk, Hammerhead, Carrier };

struct FactionProfile {
    std::string id;
    std::string displayName;

    SilhouetteProfile silhouette;
    ShapeLanguage shapeLanguage;
    FactionPalette palette;

    float armorBias = 1.0f;
    float engineBias = 1.0f;
    float weaponBias = 1.0f;

    HullArchetype preferredHull = HullArchetype::Brick;
};

class FactionDefinitions {
public:
    static FactionProfile IronDominion();
    static FactionProfile NomadContinuum();
    static FactionProfile HelixCovenant();
    static FactionProfile AshenClades();
    static FactionProfile AscendedArchive();

    static std::vector<FactionProfile> GetAllFactions();
};

} // namespace subspace
