#pragma once

#include "runtime/DeveloperModeController.h"
#include "runtime/PlayerController.h"
#include "runtime/RuntimeSession.h"
#include "runtime/RuntimeWorld.h"

#include <string>

namespace subspace {

class GameRuntime {
public:
    void Initialize();
    void Shutdown();
    void Tick(float deltaSeconds);

    DeveloperCommandExecution ExecuteDeveloperCommandLine(const std::string& line);

    RuntimeSession& GetSession() { return _session; }
    const RuntimeSession& GetSession() const { return _session; }
    RuntimeWorld& GetWorld() { return _world; }
    const RuntimeWorld& GetWorld() const { return _world; }
    PlayerController& GetPlayerController() { return _playerController; }
    const PlayerController& GetPlayerController() const { return _playerController; }
    DeveloperModeController& GetDeveloperMode() { return _developerMode; }
    const DeveloperModeController& GetDeveloperMode() const { return _developerMode; }

private:
    RuntimeSession _session;
    RuntimeWorld _world;
    PlayerController _playerController;
    DeveloperModeController _developerMode;
};

} // namespace subspace
