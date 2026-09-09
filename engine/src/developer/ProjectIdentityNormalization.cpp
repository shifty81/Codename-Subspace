#include "developer/ProjectIdentityNormalization.h"

namespace subspace {

std::vector<ProjectIdentityMarker> ProjectIdentityNormalization::DefaultMarkers()
{
    return {
        {"AvorionLike", "reference/csharp-prototype", "quarantine old C# prototype identity"},
        {"X4", "Subspace", "replace direct comparison marker with neutral project term where appropriate"},
        {"EVEInspired", "Subspace", "replace direct inspiration marker with original mechanic name"},
        {"NovaForge", "Subspace", "keep only where documented as historical reference"},
    };
}

bool ProjectIdentityNormalization::IsLegacyMarker(const std::string& text)
{
    for (const auto& marker : DefaultMarkers()) {
        if (text.find(marker.marker) != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace subspace
