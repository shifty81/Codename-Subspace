#include "core/persistence/HomeSystemSaveGame.h"

#include <sstream>

namespace subspace {

HomeSystemSaveSnapshot CreateDefaultHomeSystemSave(std::uint32_t seed) {
    HomeSystemSaveSnapshot snapshot;
    snapshot.home = CreateDefaultHomeSolarSystem(seed);
    snapshot.factory = CreateStarterHomeFactoryNetwork(snapshot.home);
    snapshot.shipyard = CreateStarterShipyardProgression();
    ExpeditionRunConfig runConfig;
    runConfig.runId = "last-dev-run";
    runConfig.seed = seed + 9001u;
    runConfig.objective = ExpeditionObjectiveType::MiningSurvey;
    runConfig.depth = 1;
    snapshot.lastRun = CreateExpeditionRun(runConfig);
    return snapshot;
}

std::string SerializeHomeSystemSaveSnapshot(const HomeSystemSaveSnapshot& snapshot) {
    // Deliberately simple line format until the final JSON save schema is locked.
    std::ostringstream out;
    out << "saveId=" << snapshot.saveId << "\n";
    out << "version=" << snapshot.saveVersion << "\n";
    out << "homeId=" << snapshot.home.id << "\n";
    out << "homeSeed=" << snapshot.home.seed << "\n";
    out << "structures=" << snapshot.home.structures.size() << "\n";
    out << "zones=" << snapshot.home.buildZones.size() << "\n";
    out << "shipyardLevel=" << snapshot.shipyard.shipyardLevel << "\n";
    out << "researchData=" << snapshot.shipyard.researchData << "\n";
    out << "lastRun=" << snapshot.lastRun.config.runId << "\n";
    out << "lastRunState=" << ExpeditionRunStateName(snapshot.lastRun.state) << "\n";
    return out.str();
}

HomeSystemSaveSnapshot DeserializeHomeSystemSaveSnapshot(const std::string& text) {
    // Temporary compatible parser: preserve a valid home save even when future fields are unknown.
    HomeSystemSaveSnapshot snapshot = CreateDefaultHomeSystemSave();
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        if (key == "saveId") snapshot.saveId = value;
        else if (key == "homeId") snapshot.home.id = value;
        else if (key == "lastRun") snapshot.lastRun.config.runId = value;
        else if (key == "version") snapshot.saveVersion = std::stoi(value);
        else if (key == "homeSeed") snapshot.home.seed = static_cast<std::uint32_t>(std::stoul(value));
        else if (key == "shipyardLevel") snapshot.shipyard.shipyardLevel = std::stoi(value);
        else if (key == "researchData") snapshot.shipyard.researchData = std::stoi(value);
    }
    return snapshot;
}

std::string HomeSystemSaveSummary(const HomeSystemSaveSnapshot& snapshot) {
    std::ostringstream out;
    out << snapshot.saveId << " v" << snapshot.saveVersion
        << " | " << HomeSolarSystemSummary(snapshot.home)
        << " | " << ShipyardProgressionSummary(snapshot.shipyard)
        << " | " << ExpeditionRunSummary(snapshot.lastRun);
    return out.str();
}

} // namespace subspace
