#include "developer/ui/DeveloperOverlay.h"
#include "runtime/GameRuntime.h"

#include <cassert>
#include <iostream>

using namespace subspace;

int main()
{
    GameRuntime runtime;
    runtime.Initialize();

    DeveloperOverlay overlay;
    overlay.SetCommandExecutor([&runtime](const std::string& line) {
        return runtime.ExecuteDeveloperCommandLine(line);
    });
    overlay.SetStatusProvider([&runtime]() {
        const auto& developerMode = runtime.GetDeveloperMode();
        const auto& session = developerMode.GetEditingLayer().GetSession();

        DeveloperOverlayStatus status;
        status.developerModeEnabled = developerMode.IsEnabled();
        status.canUndo = session.CanUndo();
        status.canRedo = session.CanRedo();
        status.dirtyEditCount = session.GetDirtyEditCount();
        status.lastMessage = developerMode.GetEditingLayer().GetLastResult().message;
        return status;
    });

    overlay.SetVisible(true);
    auto drawList = overlay.BuildDrawList(1280.0f, 720.0f);
    assert(!drawList.Empty());

    DeveloperCommandExecution modeOn = overlay.SubmitCommand("dev.mode.on");
    assert(modeOn.success);
    assert(runtime.GetDeveloperMode().IsEnabled());

    DeveloperCommandExecution selectShip = overlay.SubmitCommand("ship.select id=active");
    assert(selectShip.success);

    DeveloperCommandExecution placeBlock = overlay.SubmitCommand("ship.block.place ship=active x=1 y=2 z=3 type=hull material=steel");
    assert(placeBlock.accepted);
    assert(placeBlock.success);
    assert(runtime.GetDeveloperMode().GetEditingLayer().GetSession().CanUndo());

    DeveloperCommandExecution undo = overlay.SubmitCommand("dev.undo");
    assert(undo.accepted);
    assert(undo.success);

    DeveloperCommandExecution redo = overlay.SubmitCommand("dev.redo");
    assert(redo.accepted);
    assert(redo.success);

    overlay.ToggleVisible();
    assert(!overlay.IsVisible());
    drawList = overlay.BuildDrawList(1280.0f, 720.0f);
    assert(drawList.Empty());

    runtime.Shutdown();
    std::cout << "DeveloperOverlay console smoke test passed." << std::endl;
    return 0;
}
