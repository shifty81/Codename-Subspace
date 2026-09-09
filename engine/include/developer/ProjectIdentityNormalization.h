#pragma once

#include <string>
#include <vector>

namespace subspace {

struct ProjectIdentityMarker {
    std::string marker;
    std::string replacement;
    std::string action;
};

class ProjectIdentityNormalization {
public:
    static std::vector<ProjectIdentityMarker> DefaultMarkers();
    static bool IsLegacyMarker(const std::string& text);
};

} // namespace subspace
