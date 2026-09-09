#pragma once

#include "developer/ui/DeveloperConsole.h"
#include "developer/ui/DeveloperOverlayState.h"

#include <functional>
#include <string>

namespace subspace {

class DeveloperOverlay {
public:
    using StatusProvider = std::function<DeveloperOverlayStatus()>;

    DeveloperOverlay();

    void SetCommandExecutor(DeveloperConsole::CommandLineExecutor executor);
    void SetStatusProvider(StatusProvider provider);

    void SetVisible(bool visible);
    void ToggleVisible();
    bool IsVisible() const { return _visible; }

    DeveloperCommandExecution SubmitCommand(const std::string& line);
    DeveloperOverlayDrawList BuildDrawList(float viewportWidth, float viewportHeight) const;

    DeveloperConsole& GetConsole() { return _console; }
    const DeveloperConsole& GetConsole() const { return _console; }

private:
    DeveloperOverlayStatus GetStatus() const;
    void AddPrimitive(DeveloperOverlayDrawList& list,
                      DeveloperOverlayPrimitiveType type,
                      float x,
                      float y,
                      float width,
                      float height,
                      std::string text = {}) const;

    bool _visible = false;
    DeveloperConsole _console;
    StatusProvider _statusProvider;
};

} // namespace subspace
