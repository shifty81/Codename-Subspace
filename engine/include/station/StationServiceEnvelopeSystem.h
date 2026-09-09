#pragma once

#include "station/StationEcologySystem.h"

namespace subspace {

struct StationServiceEnvelopeProfile {
    float radius = 0.0f;
    float repairPerSecond = 0.0f;
    float shieldPerSecond = 0.0f;
    float refuelPerSecond = 0.0f;
    bool defenseCoverage = false;
    bool tractorAssistance = false;
};

class StationServiceEnvelopeSystem {
public:
    static StationServiceEnvelopeProfile Build(StationArchetype archetype,bool authorized);
};

} // namespace subspace
