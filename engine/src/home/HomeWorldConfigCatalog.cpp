#include "home/HomeWorldConfigCatalog.h"

#include <sstream>

namespace subspace {

std::vector<HomeWorldConfigPreset> CreateHomeWorldConfigPresets() {
    HomeWorldConfig relaxed;
    relaxed.safetyMode = HomeSafetyMode::Safe;
    relaxed.persistHomeSystem = true;
    relaxed.allowOfflineProduction = true;
    relaxed.automationLimit = 24;

    HomeWorldConfig standard;
    standard.safetyMode = HomeSafetyMode::Safe;
    standard.persistHomeSystem = true;
    standard.allowOfflineProduction = false;
    standard.automationLimit = 12;

    HomeWorldConfig frontier;
    frontier.safetyMode = HomeSafetyMode::CosmeticThreats;
    frontier.persistHomeSystem = true;
    frontier.allowHomeRaids = true;
    frontier.allowHomeStructureDamage = false;
    frontier.allowOfflineProduction = true;
    frontier.automationLimit = 16;

    HomeWorldConfig hardcore;
    hardcore.safetyMode = HomeSafetyMode::Hardcore;
    hardcore.persistHomeSystem = true;
    hardcore.allowHomeRaids = true;
    hardcore.allowHomeStructureDamage = true;
    hardcore.allowOfflineProduction = false;
    hardcore.automationLimit = 8;

    return {
        {"relaxed", "Relaxed Builder", "Safe persistent home with generous automation.", relaxed},
        {"standard", "Standard", "Safe persistent home; risk lives in expedition systems.", standard},
        {"frontier", "Frontier", "Home remains persistent but can show threat events.", frontier},
        {"hardcore", "Hardcore", "Home can suffer real damage and automation is constrained.", hardcore},
    };
}

HomeWorldConfig CreateHomeWorldConfigById(const std::string& id) {
    for (const auto& preset : CreateHomeWorldConfigPresets()) {
        if (preset.id == id) return preset.config;
    }
    return HomeWorldConfig{};
}

std::string HomeWorldConfigSummary(const HomeWorldConfig& config) {
    std::ostringstream stream;
    stream << "HomeConfig safety=" << HomeSafetyModeName(config.safetyMode)
           << " persistent=" << (config.persistHomeSystem ? "yes" : "no")
           << " raids=" << (config.allowHomeRaids ? "yes" : "no")
           << " damage=" << (config.allowHomeStructureDamage ? "yes" : "no")
           << " offline=" << (config.allowOfflineProduction ? "yes" : "no")
           << " automationLimit=" << config.automationLimit;
    return stream.str();
}

} // namespace subspace
