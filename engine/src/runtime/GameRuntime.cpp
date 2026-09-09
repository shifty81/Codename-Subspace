#include "runtime/GameRuntime.h"

namespace subspace {

void GameRuntime::Initialize()
{
    _session.StartNew();
    _developerMode.InitializeDefaults();
}

void GameRuntime::Shutdown()
{
    _session.Shutdown();
}

void GameRuntime::Tick(float deltaSeconds)
{
    _session.Tick(deltaSeconds);
    _developerMode.Tick(deltaSeconds);
}

DeveloperCommandExecution GameRuntime::ExecuteDeveloperCommandLine(const std::string& line)
{
    return _developerMode.ExecuteCommandLine(line);
}

} // namespace subspace
