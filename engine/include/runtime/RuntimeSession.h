#pragma once

#include "runtime/RuntimeMode.h"

#include <cstdint>
#include <string>

namespace subspace {

class RuntimeSession {
public:
    void StartNew(std::string sessionName = "Subspace Development Session");
    void Shutdown();

    RuntimeMode GetMode() const { return _mode; }
    void SetMode(RuntimeMode mode) { _mode = mode; }

    const std::string& GetSessionName() const { return _sessionName; }
    std::uint64_t GetFrameIndex() const { return _frameIndex; }
    double GetElapsedSeconds() const { return _elapsedSeconds; }
    bool IsActive() const { return _active; }

    void Tick(float deltaSeconds);

private:
    bool _active = false;
    RuntimeMode _mode = RuntimeMode::None;
    std::string _sessionName;
    std::uint64_t _frameIndex = 0;
    double _elapsedSeconds = 0.0;
};

} // namespace subspace
