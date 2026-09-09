#pragma once

#include <string>
#include <vector>

namespace subspace {

struct RailTravelPresentationSegment {
    std::string segmentId;
    std::string label;
    float start01 = 0.0f;
    float end01 = 1.0f;
    float resourceYield = 0.0f;
    float hazard = 0.0f;
};

struct RailTravelPresentationModel {
    std::string routeName;
    float progress01 = 0.0f;
    std::vector<RailTravelPresentationSegment> segments;
    std::vector<std::string> alerts;
};

RailTravelPresentationModel CreateRailTravelPresentationModel(const std::string& routeName, float progress01, int richness, int hazards);
RailTravelPresentationSegment ActiveRailTravelSegment(const RailTravelPresentationModel& model);

} // namespace subspace
