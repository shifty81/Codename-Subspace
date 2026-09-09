#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class SandboxWorkspaceMode { Flight, GalaxyMap, SystemMap, PlanetSurvey, PlanetaryManufacturing, StationBuilder, ShipBuilder, HangarFitting, MarketContracts, Exploration, FleetCorporation };

class SandboxWorkspaceSystem {
public:
    SandboxWorkspaceMode Mode() const { return mode_; }
    void Open(SandboxWorkspaceMode mode) { mode_=mode; }
    void Close() { mode_=SandboxWorkspaceMode::Flight; }
    void Toggle(SandboxWorkspaceMode mode) { mode_=mode_==mode?SandboxWorkspaceMode::Flight:mode; }
    bool IsOverlayOpen() const { return mode_!=SandboxWorkspaceMode::Flight; }
    std::string Title() const;
    std::vector<std::string> Actions() const;
private:
    SandboxWorkspaceMode mode_=SandboxWorkspaceMode::Flight;
};

} // namespace subspace
