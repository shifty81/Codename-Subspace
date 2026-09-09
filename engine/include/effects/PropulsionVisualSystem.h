#pragma once

#include "content/ShipyardModuleSystem.h"

namespace subspace {

enum class PropulsionVisualLod { Near, Mid, Far };

struct PropulsionVisualProfile {
    float nozzleGlow = 0.0f;
    float coreLength = 0.0f;
    float bodyLength = 0.0f;
    float haloWidth = 0.0f;
    float flicker = 0.0f;
    int sparkBudget = 0;
    bool ribbon = false;
    bool distortion = false;
};

class PropulsionVisualSystem {
public:
    static PropulsionVisualProfile Evaluate(ShipyardModuleSemantic semantic,float activity,
                                            PropulsionVisualLod lod,bool boost,bool vectorDrive);
};

} // namespace subspace
