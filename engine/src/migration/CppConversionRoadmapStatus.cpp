#include "migration/CppConversionRoadmapStatus.h"

namespace subspace {

CppConversionRoadmapStatus CreateCurrentCppConversionRoadmapStatus() {
    CppConversionRoadmapStatus status;
    status.areas = {
        {"client", "Playable C++ client", 55, true},
        {"ships", "Ship blocks/modules/parts", 50, true},
        {"home", "Persistent home system", 35, true},
        {"factory", "Home factory automation", 25, true},
        {"travel", "On-rails interstellar travel", 30, true},
        {"combat", "Combat/weapons/shields", 20, true},
        {"economy", "Stations/economy/trade", 25, true},
        {"factions", "Factions/encounters/AI", 20, true},
        {"save", "Persistence/save-load", 15, true},
        {"content", "Content/data normalization", 20, true},
        {"legacy", "Legacy C# audit closure", 10, true},
    };
    return status;
}

int EstimateOverallCppConversionPercent(const CppConversionRoadmapStatus& status) {
    if (status.areas.empty()) return 0;
    int total = 0;
    for (const auto& area : status.areas) total += area.percentComplete;
    return total / static_cast<int>(status.areas.size());
}

std::vector<std::string> CppConversionBlockingAreas(const CppConversionRoadmapStatus& status) {
    std::vector<std::string> blockers;
    for (const auto& area : status.areas) {
        if (area.blocksCSharpRetirement && area.percentComplete < 100) blockers.push_back(area.label);
    }
    return blockers;
}

} // namespace subspace
