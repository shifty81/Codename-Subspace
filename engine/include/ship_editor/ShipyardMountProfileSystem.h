#pragma once

#include "content/ShipyardModuleSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct ShipyardMountSurface {
    std::string face;
    Vector3 point{};
    Vector3 normal{};
    float supportingArea = 0.0f;
    float confidence = 0.0f;
    bool valid = false;
};

struct ShipyardMountProfile {
    std::string moduleId;
    ShipyardMountSurface primaryRoot{};
    std::vector<ShipyardMountSurface> alternates;
    std::string placementRole;
    float insertionDepthMeters = 0.05f;
    float minimumClearanceMeters = 0.10f;
    bool generatorEligible = false;
    bool pairedPlacement = false;
    bool valid = false;
    std::string provenance;
};

class ShipyardMountProfileSystem {
public:
    static ShipyardMountProfile Build(const ShipyardModuleRecord& record);
    static ShipyardMountSurface SurfaceForFace(const ShipyardModuleRecord& record,
                                               std::string face);
    static ShipyardMountSurface LargestFlatSurface(const ShipyardModuleRecord& record);
    static bool IsBroadRootStructuralPart(const ShipyardModuleRecord& record);
};

} // namespace subspace
