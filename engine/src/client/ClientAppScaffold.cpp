#include "client/ClientAppScaffold.h"

#include <sstream>

namespace subspace {

const char* ClientMajorModeName(ClientMajorMode mode) {
    switch (mode) {
        case ClientMajorMode::ExpeditionFlight: return "ExpeditionFlight";
        case ClientMajorMode::HomeSolarSystem: return "HomeSolarSystem";
        case ClientMajorMode::HomeBuildMode: return "HomeBuildMode";
        case ClientMajorMode::InterstellarRailTravel: return "InterstellarRailTravel";
        case ClientMajorMode::ShipBuilder: return "ShipBuilder";
        case ClientMajorMode::DeveloperOverlay: return "DeveloperOverlay";
        default: return "Unknown";
    }
}

ClientModeTransition SwitchClientMode(ClientAppScaffoldState& state, ClientMajorMode mode, const std::string& reason) {
    ClientModeTransition transition;
    transition.from = state.activeMode;
    transition.to = mode;
    transition.reason = reason;
    state.activeMode = mode;
    state.buildModeEnabled = (mode == ClientMajorMode::HomeBuildMode || mode == ClientMajorMode::ShipBuilder);
    state.travelOverlayOpen = (mode == ClientMajorMode::InterstellarRailTravel);
    state.developerOverlayOpen = (mode == ClientMajorMode::DeveloperOverlay) ? true : state.developerOverlayOpen;
    state.transitions.push_back(transition);
    return transition;
}

std::vector<std::string> GetClientDecompositionTargets() {
    return {
        "ClientApp",
        "ClientInput",
        "ClientWorldState",
        "ClientRendererGdi",
        "ClientHudView",
        "ClientHomeView",
        "ClientSolarSystemView",
        "ClientTravelView",
        "ClientShipBuilderView",
        "ClientDevConsoleView"
    };
}

std::string ClientAppScaffoldSummary(const ClientAppScaffoldState& state) {
    std::ostringstream ss;
    ss << "mode=" << ClientMajorModeName(state.activeMode)
       << " transitions=" << state.transitions.size()
       << " buildMode=" << (state.buildModeEnabled ? "on" : "off")
       << " travel=" << (state.travelOverlayOpen ? "open" : "closed");
    return ss.str();
}

} // namespace subspace
