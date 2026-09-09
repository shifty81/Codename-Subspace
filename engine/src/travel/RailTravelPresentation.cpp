#include "travel/RailTravelPresentation.h"

#include <algorithm>

namespace subspace {

RailTravelPresentationModel CreateRailTravelPresentationModel(const std::string& routeName, float progress01, int richness, int hazards) {
    RailTravelPresentationModel model;
    model.routeName = routeName;
    model.progress01 = std::max(0.0f, std::min(1.0f, progress01));
    const float yieldScale = static_cast<float>(std::max(0, richness));
    const float hazardScale = static_cast<float>(std::max(0, hazards));
    model.segments.push_back({"departure", "Departure Burn", 0.0f, 0.18f, 0.0f, 0.1f * hazardScale});
    model.segments.push_back({"harvest", "Debris Harvest Lane", 0.18f, 0.52f, 2.0f * yieldScale, 0.2f * hazardScale});
    model.segments.push_back({"hazard", "Unstable Drift Band", 0.52f, 0.76f, 1.0f * yieldScale, 0.7f * hazardScale});
    model.segments.push_back({"approach", "Destination Approach", 0.76f, 1.0f, 0.5f * yieldScale, 0.3f * hazardScale});
    if (hazards > richness) model.alerts.push_back("Route hazard exceeds expected yield; improve shielding or choose a safer path.");
    return model;
}

RailTravelPresentationSegment ActiveRailTravelSegment(const RailTravelPresentationModel& model) {
    for (const auto& segment : model.segments) {
        if (model.progress01 >= segment.start01 && model.progress01 <= segment.end01) return segment;
    }
    if (!model.segments.empty()) return model.segments.back();
    return {};
}

} // namespace subspace
