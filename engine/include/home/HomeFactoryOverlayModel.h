#pragma once

#include <string>
#include <vector>

namespace subspace {

struct HomeFactoryOverlayLine {
    std::string id;
    std::string label;
    std::string value;
    bool warning = false;
};

struct HomeFactoryOverlayModel {
    std::string title = "Home Surface";
    std::vector<HomeFactoryOverlayLine> lines;
};

HomeFactoryOverlayModel BuildHomeFactoryOverlay(float powerNet, float logisticsNet, float productionNet, float storedOre);
std::string HomeFactoryOverlayCompactSummary(const HomeFactoryOverlayModel& model);

} // namespace subspace
