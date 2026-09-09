#include "home/HomeFactoryOverlayModel.h"

#include <sstream>

namespace subspace {

static std::string Num(float value) {
    std::ostringstream out;
    out.precision(1);
    out << std::fixed << value;
    return out.str();
}

HomeFactoryOverlayModel BuildHomeFactoryOverlay(float powerNet, float logisticsNet, float productionNet, float storedOre) {
    HomeFactoryOverlayModel model;
    model.lines.push_back({"power", "Power Net", Num(powerNet), powerNet < 0.0f});
    model.lines.push_back({"logistics", "Logistics", Num(logisticsNet), logisticsNet < 0.0f});
    model.lines.push_back({"production", "Production", Num(productionNet) + "/m", productionNet <= 0.0f});
    model.lines.push_back({"ore", "Stored Ore", Num(storedOre), storedOre <= 0.0f});
    return model;
}

std::string HomeFactoryOverlayCompactSummary(const HomeFactoryOverlayModel& model) {
    std::ostringstream out;
    out << model.title;
    for (const auto& line : model.lines) {
        out << " | " << line.label << ": " << line.value;
    }
    return out.str();
}

} // namespace subspace
