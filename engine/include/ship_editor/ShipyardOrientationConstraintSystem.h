#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <string>

namespace subspace {

struct ShipyardOrientationRule {
    bool allowVertical = true;
    bool matchShipForward = false;
    bool matchShipDorsal = false;
    float maximumRollDegrees = 180.0f;
    bool mirrorRecommended = false;
};

struct ShipyardOrientationValidation {
    bool valid = true;
    std::string warning;
};

class ShipyardOrientationConstraintSystem {
public:
    static ShipyardOrientationRule RuleFor(const ShipyardModuleRecord& module);
    static ShipyardOrientationValidation Validate(const ShipyardModuleRecord& module,const VisualModulePlacement& placement);
    static void Normalize(const ShipyardModuleRecord& module,VisualModulePlacement& placement);
};

} // namespace subspace
