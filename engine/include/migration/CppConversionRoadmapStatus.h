#pragma once

#include <string>
#include <vector>

namespace subspace {

struct CppConversionRoadmapArea {
    std::string areaId;
    std::string label;
    int percentComplete = 0;
    bool blocksCSharpRetirement = true;
};

struct CppConversionRoadmapStatus {
    std::vector<CppConversionRoadmapArea> areas;
};

CppConversionRoadmapStatus CreateCurrentCppConversionRoadmapStatus();
int EstimateOverallCppConversionPercent(const CppConversionRoadmapStatus& status);
std::vector<std::string> CppConversionBlockingAreas(const CppConversionRoadmapStatus& status);

} // namespace subspace
