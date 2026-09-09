#include "home/HomeDysonProgression.h"

#include <algorithm>

namespace subspace {

const char* HomeDysonStageName(HomeDysonStage stage) {
    switch (stage) {
    case HomeDysonStage::None: return "None";
    case HomeDysonStage::SolarPanels: return "Solar Panels";
    case HomeDysonStage::CollectorSatellites: return "Collector Satellites";
    case HomeDysonStage::RelayNetwork: return "Relay Network";
    case HomeDysonStage::SwarmNodes: return "Swarm Nodes";
    case HomeDysonStage::FabricationRing: return "Fabrication Ring";
    case HomeDysonStage::ShellSegments: return "Shell Segments";
    }
    return "Unknown";
}

HomeDysonProgressionState AdvanceHomeDysonProgression(HomeDysonProgressionState state, float investedAlloys, float investedElectronics) {
    const float contribution = std::max(0.0f, investedAlloys) * 0.001f + std::max(0.0f, investedElectronics) * 0.0015f;
    state.buildProgress01 = std::min(1.0f, state.buildProgress01 + contribution);
    if (state.buildProgress01 >= 1.0f && state.stage != HomeDysonStage::ShellSegments) {
        state.stage = static_cast<HomeDysonStage>(static_cast<int>(state.stage) + 1);
        state.buildProgress01 = 0.0f;
    }
    state.collectorCount += static_cast<int>(investedAlloys / 50.0f);
    state.relayCount += static_cast<int>(investedElectronics / 60.0f);
    state.energyOutput = 5.0f + state.collectorCount * 1.5f + state.relayCount * 0.8f + static_cast<int>(state.stage) * 10.0f;
    return state;
}

std::vector<std::string> HomeDysonNextRequirements(HomeDysonStage stage) {
    switch (stage) {
    case HomeDysonStage::None: return {"200 alloys", "120 electronics", "shipyard tier 1"};
    case HomeDysonStage::SolarPanels: return {"collector satellite blueprint", "500 alloys", "300 electronics"};
    case HomeDysonStage::CollectorSatellites: return {"relay node research", "900 alloys", "600 electronics"};
    case HomeDysonStage::RelayNetwork: return {"swarm control AI", "rare conductors"};
    case HomeDysonStage::SwarmNodes: return {"orbital fabrication ring", "high-grade reactor cores"};
    case HomeDysonStage::FabricationRing: return {"shell segment authorization", "exotic matter"};
    case HomeDysonStage::ShellSegments: return {"stellar harness maintenance"};
    }
    return {};
}

} // namespace subspace
