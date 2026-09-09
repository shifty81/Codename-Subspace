#include "developer/ui/DeveloperOverlay.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace subspace {

DeveloperOverlay::DeveloperOverlay()
{
    _console.AddOutput("Subspace developer console ready.");
}

void DeveloperOverlay::SetCommandExecutor(DeveloperConsole::CommandLineExecutor executor)
{
    _console.SetExecutor(std::move(executor));
}

void DeveloperOverlay::SetStatusProvider(StatusProvider provider)
{
    _statusProvider = std::move(provider);
}

void DeveloperOverlay::SetVisible(bool visible)
{
    _visible = visible;
}

void DeveloperOverlay::ToggleVisible()
{
    _visible = !_visible;
}

DeveloperCommandExecution DeveloperOverlay::SubmitCommand(const std::string& line)
{
    return _console.Submit(line);
}

DeveloperOverlayDrawList DeveloperOverlay::BuildDrawList(float viewportWidth, float viewportHeight) const
{
    DeveloperOverlayDrawList list;
    if (!_visible) {
        return list;
    }

    const float panelWidth = std::max(420.0f, viewportWidth * 0.52f);
    const float panelHeight = std::max(260.0f, viewportHeight * 0.42f);
    const float margin = 16.0f;
    const float x = margin;
    const float y = margin;
    const float lineHeight = 18.0f;

    AddPrimitive(list, DeveloperOverlayPrimitiveType::Panel, x, y, panelWidth, panelHeight, "Developer Overlay");

    DeveloperOverlayStatus status = GetStatus();
    std::ostringstream statusText;
    statusText << (status.developerModeEnabled ? "DEV ON" : "DEV OFF")
               << " | dirty=" << status.dirtyEditCount
               << " | undo=" << (status.canUndo ? "yes" : "no")
               << " | redo=" << (status.canRedo ? "yes" : "no");
    AddPrimitive(list, DeveloperOverlayPrimitiveType::StatusPill, x + 12.0f, y + 12.0f, panelWidth - 24.0f, lineHeight, statusText.str());

    const float outputTop = y + 40.0f;
    const float outputLeft = x + 12.0f;
    const float outputWidth = panelWidth - 24.0f;
    const float inputHeight = 28.0f;
    const float inputY = y + panelHeight - inputHeight - 12.0f;
    AddPrimitive(list, DeveloperOverlayPrimitiveType::Divider, outputLeft, inputY - 8.0f, outputWidth, 1.0f, {});

    const auto& lines = _console.GetOutputLines();
    const std::size_t visibleLines = static_cast<std::size_t>(std::max(1.0f, (inputY - outputTop - 10.0f) / lineHeight));
    const std::size_t start = lines.size() > visibleLines ? lines.size() - visibleLines : 0;
    float lineY = outputTop;
    for (std::size_t i = start; i < lines.size(); ++i) {
        AddPrimitive(list, DeveloperOverlayPrimitiveType::Text, outputLeft, lineY, outputWidth, lineHeight, lines[i].text);
        lineY += lineHeight;
    }

    AddPrimitive(list, DeveloperOverlayPrimitiveType::InputBox, outputLeft, inputY, outputWidth, inputHeight, "> " + _console.GetInput());
    return list;
}

DeveloperOverlayStatus DeveloperOverlay::GetStatus() const
{
    if (_statusProvider) {
        DeveloperOverlayStatus status = _statusProvider();
        status.visible = _visible;
        return status;
    }
    DeveloperOverlayStatus status;
    status.visible = _visible;
    status.lastMessage = _console.GetLastExecution().message;
    return status;
}

void DeveloperOverlay::AddPrimitive(DeveloperOverlayDrawList& list,
                                    DeveloperOverlayPrimitiveType type,
                                    float x,
                                    float y,
                                    float width,
                                    float height,
                                    std::string text) const
{
    DeveloperOverlayPrimitive primitive;
    primitive.type = type;
    primitive.x = x;
    primitive.y = y;
    primitive.width = width;
    primitive.height = height;
    primitive.text = std::move(text);
    list.primitives.push_back(std::move(primitive));
}

} // namespace subspace
