#pragma once

#include <string>
#include <vector>

namespace subspace {

struct HudMetricLine {
    std::string label;
    std::string value;
    bool warning = false;
};

struct ClientHudPanelModel {
    std::string title;
    std::vector<HudMetricLine> lines;
};

ClientHudPanelModel BuildClientHudPanel(const std::string& sectorId,
                                        int credits,
                                        int cargoUnits,
                                        float hull,
                                        float speed,
                                        const std::string& scannerSummary);
std::string ClientHudPanelSummary(const ClientHudPanelModel& panel);

} // namespace subspace
