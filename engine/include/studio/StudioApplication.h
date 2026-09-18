#pragma once
#include "application/NativeBattlefieldRenderer.h"
#include "input/InputState.h"
#include "platform/NativeWindow.h"
#include "rendering/StrategicCamera.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "studio/StudioDocumentStore.h"
#include "ship_editor/ShipyardDockPointerSystem.h"
#include <cstdint>
#include <filesystem>

namespace subspace {
// Dedicated authoring controller: no gameplay frontend, no player entity and
// no game-owned application lifecycle. Shares certified render/catalog services.
class StudioApplication {
public:
    StudioApplication();
    int Run(const std::filesystem::path& openFile={},std::uint64_t maxFrames=0);
private:
    void HandleInput();
    void HandleEscape();
    void RouteControl(ShipyardBuilderCommand command,int value=0);
    void SaveDocument();
    void LoadAuthoringOverrides();
    void SaveAuthoringOverrides(bool sockets);
    void RenderFrame(float elapsed);
    InputState input_;
    NativeWindow window_;
    NativeBattlefieldRenderer renderer_;
    StrategicCamera camera_;
    ShipyardBuilderSystem builder_;
    StudioDocumentStore documents_;
    GalaxySector emptySector_{};
    ShipyardDockPointerSystem dockPointer_;
    bool pendingCatalogPress_=false;
    bool catalogDragging_=false;
    bool pointerTransform_=false;
    bool suppressClick_=false;
    int pendingCatalogIndex_=-1;
    float catalogPressX_=0.0f, catalogPressY_=0.0f;
};
} // namespace subspace
