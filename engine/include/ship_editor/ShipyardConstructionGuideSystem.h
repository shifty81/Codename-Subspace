#pragma once

#include "ship_editor/ShipyardMountProfileSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipyardConstructionGuideKind { SymmetryPlane, MountPlane, SocketNormal, Clearance, DeckHeight, PlayerScale, Distance, Alignment, WeaponClearance, ThrusterExclusion };

struct ShipyardConstructionGuide {
    ShipyardConstructionGuideKind kind = ShipyardConstructionGuideKind::Alignment;
    std::string id;
    std::string label;
    Vector3 origin{};
    Vector3 direction{0,1,0};
    float extentMeters = 1.0f;
    bool temporary = true;
    bool warning = false;
};

class ShipyardConstructionGuideSystem {
public:
    static std::vector<ShipyardConstructionGuide> ForMountProfile(const ShipyardMountProfile& profile,
                                                                  const Vector3& moduleOrigin,
                                                                  bool includePlayerScale = true);
    static ShipyardConstructionGuide DistanceGuide(const Vector3& a, const Vector3& b, std::string label = {});
};

} // namespace subspace
