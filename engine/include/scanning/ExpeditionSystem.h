#pragma once

#include "scanning/ExplorationDiscoverySystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct DirectionalScanContact { std::uint64_t signatureId=0;double bearingDegrees=0;double strength=0;bool resolved=false; };
struct ExpeditionChain { std::uint64_t rootSignature=0;std::vector<std::uint64_t> stages;std::size_t currentStage=0;bool completed=false; };

class ExpeditionSystem {
public:
    std::vector<DirectionalScanContact> DirectionalScan(const std::vector<ExplorationSignature>& signatures,double centerBearing,double coneDegrees,double sensorStrength) const;
    bool AdvanceChain(ExpeditionChain& chain,std::uint64_t resolvedSignature) const;
    double ProbeTimeSeconds(double difficulty,double probeStrength) const;
};

} // namespace subspace
