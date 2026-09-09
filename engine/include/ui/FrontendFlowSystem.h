#pragma once

#include "ships/ShipConstructionSystem.h"
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class FrontendScreen { MainMenu, NewSandbox, LoadSandbox, Settings, Credits, StartingShip, StartingLoadout, StationHangar, InGame };
enum class StarterCareer { Prospector, Scrapper, Pathfinder, Defender, Custom };

struct NewSandboxConfig { std::string commanderName="Cadet"; std::string corporationName="Independent Operations"; std::uint64_t galaxySeed=1; bool storyEnabled=true; bool coopEnabled=true; };
struct StarterSelection { StarterCareer career=StarterCareer::Prospector; ModularShipDesign design; std::vector<std::string> startingEquipment; };

class FrontendFlowSystem {
public:
    FrontendScreen Screen() const { return screen_; }
    void GoTo(FrontendScreen screen) { screen_=screen; }
    bool BeginNewSandbox(const NewSandboxConfig& config);
    StarterSelection BuildStarter(StarterCareer career) const;
    bool ConfirmStarter(const StarterSelection& selection);
    const NewSandboxConfig& Config() const { return config_; }
    void SetConfig(const NewSandboxConfig& config) { config_ = config; }
    const StarterSelection& Selection() const { return selection_; }
private:
    FrontendScreen screen_=FrontendScreen::MainMenu;
    NewSandboxConfig config_{};
    StarterSelection selection_{};
};

} // namespace subspace
