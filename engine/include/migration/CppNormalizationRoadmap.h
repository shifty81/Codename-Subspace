#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class NormalizationGateStatus {
    NotStarted,
    InProgress,
    Ready,
    Complete
};

struct NormalizationGate {
    std::string id;
    std::string title;
    std::string ownerArea;
    NormalizationGateStatus status = NormalizationGateStatus::NotStarted;
    std::vector<std::string> requiredProof;
};

struct CppNormalizationRoadmap {
    std::vector<NormalizationGate> gates;
};

CppNormalizationRoadmap CreatePostPass114CppNormalizationRoadmap();
std::string NormalizationGateStatusName(NormalizationGateStatus status);
std::string CppNormalizationRoadmapSummary(const CppNormalizationRoadmap& roadmap);

} // namespace subspace
