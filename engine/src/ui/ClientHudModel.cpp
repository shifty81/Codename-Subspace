#include "ui/ClientHudModel.h"

#include <sstream>

namespace subspace {

ClientHudPanelModel BuildClientHudPanel(const std::string& sectorId,
                                        int credits,
                                        int cargoUnits,
                                        float hull,
                                        float speed,
                                        const std::string& scannerSummary)
{
    ClientHudPanelModel panel;
    panel.title = "Subspace Client HUD";
    panel.lines.push_back({"Sector", sectorId, false});
    panel.lines.push_back({"Credits", std::to_string(credits), false});
    panel.lines.push_back({"Cargo", std::to_string(cargoUnits), cargoUnits > 12});
    panel.lines.push_back({"Hull", std::to_string(static_cast<int>(hull)), hull < 35.0f});
    panel.lines.push_back({"Speed", std::to_string(static_cast<int>(speed)), false});
    panel.lines.push_back({"Scanner", scannerSummary, false});
    return panel;
}

std::string ClientHudPanelSummary(const ClientHudPanelModel& panel)
{
    std::ostringstream stream;
    stream << panel.title << " lines=" << panel.lines.size();
    return stream.str();
}

} // namespace subspace
