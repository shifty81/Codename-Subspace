#include "home/HomePowerGrid.h"

#include <algorithm>
#include <sstream>

namespace subspace {
namespace {
int StructurePowerLoad(HomeStructureType type, int tier) {
    switch (type) {
        case HomeStructureType::Extractor: return 1 * tier;
        case HomeStructureType::ConveyorHub: return 1;
        case HomeStructureType::Refinery: return 2 * tier;
        case HomeStructureType::Assembler: return 3 * tier;
        case HomeStructureType::StorageDepot: return 1;
        case HomeStructureType::DroneDepot: return 2 * tier;
        case HomeStructureType::ResearchLab: return 3 * tier;
        case HomeStructureType::ShipyardBay: return 4 * tier;
        case HomeStructureType::SubspaceAnchor: return 8 * tier;
        default: return 0;
    }
}
}

int EstimateHomeStructurePowerLoad(const HomeSolarSystemState& home) {
    int load = 0;
    for (const auto& structure : home.structures) {
        load += StructurePowerLoad(structure.type, std::max(1, structure.tier));
    }
    return load;
}

HomePowerGridReport AnalyzeHomePowerGrid(const HomeSolarSystemState& home, float stellarIntensity) {
    HomePowerGridReport report;
    report.solarEfficiency = std::max(0.1f, stellarIntensity);
    report.solarCollectors = CountHomeStructures(home, HomeStructureType::SolarCollector);
    report.dysonNodes = CountHomeStructures(home, HomeStructureType::DysonSwarmNode);
    const int solarPower = static_cast<int>(static_cast<float>(report.solarCollectors * 5) * report.solarEfficiency);
    const int dysonPower = static_cast<int>(static_cast<float>(report.dysonNodes * 18) * report.solarEfficiency);
    report.generation = EstimateHomePowerGeneration(home) + solarPower + dysonPower;
    report.structureLoad = EstimateHomeStructurePowerLoad(home);
    report.stored = home.storedPower;
    report.surplus = report.generation + report.stored - report.structureLoad;
    report.stable = report.surplus >= 0;
    return report;
}

std::string HomePowerGridSummary(const HomePowerGridReport& report) {
    std::ostringstream stream;
    stream << "Power gen=" << report.generation << " load=" << report.structureLoad
           << " stored=" << report.stored << " surplus=" << report.surplus
           << " collectors=" << report.solarCollectors << " dyson=" << report.dysonNodes
           << " status=" << (report.stable ? "stable" : "deficit");
    return stream.str();
}

} // namespace subspace
