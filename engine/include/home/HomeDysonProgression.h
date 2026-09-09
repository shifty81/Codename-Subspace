#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class HomeDysonStage { None, SolarPanels, CollectorSatellites, RelayNetwork, SwarmNodes, FabricationRing, ShellSegments };

struct HomeDysonProgressionState {
    HomeDysonStage stage = HomeDysonStage::None;
    float energyOutput = 0.0f;
    float buildProgress01 = 0.0f;
    int collectorCount = 0;
    int relayCount = 0;
};

const char* HomeDysonStageName(HomeDysonStage stage);
HomeDysonProgressionState AdvanceHomeDysonProgression(HomeDysonProgressionState state, float investedAlloys, float investedElectronics);
std::vector<std::string> HomeDysonNextRequirements(HomeDysonStage stage);

} // namespace subspace
