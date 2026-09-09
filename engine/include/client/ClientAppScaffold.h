#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ClientMajorMode {
    ExpeditionFlight,
    HomeSolarSystem,
    HomeBuildMode,
    InterstellarRailTravel,
    ShipBuilder,
    DeveloperOverlay
};

struct ClientModeTransition {
    ClientMajorMode from = ClientMajorMode::ExpeditionFlight;
    ClientMajorMode to = ClientMajorMode::ExpeditionFlight;
    std::string reason;
};

struct ClientAppScaffoldState {
    ClientMajorMode activeMode = ClientMajorMode::ExpeditionFlight;
    bool developerOverlayOpen = false;
    bool buildModeEnabled = false;
    bool travelOverlayOpen = false;
    std::vector<ClientModeTransition> transitions;
};

const char* ClientMajorModeName(ClientMajorMode mode);
ClientModeTransition SwitchClientMode(ClientAppScaffoldState& state, ClientMajorMode mode, const std::string& reason);
std::vector<std::string> GetClientDecompositionTargets();
std::string ClientAppScaffoldSummary(const ClientAppScaffoldState& state);

} // namespace subspace
