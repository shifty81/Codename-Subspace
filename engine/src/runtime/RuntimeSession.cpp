#include "runtime/RuntimeSession.h"

#include <utility>

namespace subspace {

void RuntimeSession::StartNew(std::string sessionName)
{
    _active = true;
    _mode = RuntimeMode::Playing;
    _sessionName = std::move(sessionName);
    _frameIndex = 0;
    _elapsedSeconds = 0.0;
}

void RuntimeSession::Shutdown()
{
    _active = false;
    _mode = RuntimeMode::ShuttingDown;
}

void RuntimeSession::Tick(float deltaSeconds)
{
    if (!_active) {
        return;
    }
    ++_frameIndex;
    _elapsedSeconds += deltaSeconds;
}

} // namespace subspace
