#pragma once
#include <string>
#include <vector>

namespace subspace {

struct PerformanceReferenceMetric {
    std::string id;
    std::string description;
    bool captureRequired = true;
};

struct PerformanceReferenceProfile {
    std::string baselineId;
    std::string certifiedGateId;
    std::vector<PerformanceReferenceMetric> metrics;
};

class PerformanceReferenceProfiles {
public:
    static PerformanceReferenceProfile P921Reference();
};

} // namespace subspace
