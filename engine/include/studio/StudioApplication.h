#pragma once
#include "application/NativeBattlefieldRenderer.h"
#include "input/InputState.h"
#include "platform/NativeWindow.h"
#include "rendering/StrategicCamera.h"
#include "editor/ConstructionEditorCameraSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "studio/StudioDocumentStore.h"
#include "studio/StudioClosePolicy.h"
#include "studio/StudioExitOutcomePolicy.h"
#include "studio/StudioNativeCloseGuard.h"
#include "studio/StudioAxisGizmo.h"
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
    void RestoreGizmoConstraint();
    StudioCloseState CloseState() const;
    bool ConfirmClose();
    void RouteControl(ShipyardBuilderCommand command,int value=0);
    void SaveDocument();
    void SaveAsDocument();
    void OpenDocument();
    void NewDocument();
    void ShowDocumentError(const std::string& error);
    void LoadAuthoringOverrides();
    void SaveAuthoringOverrides(bool sockets);
    void RenderFrame(float elapsed);
    InputState input_;
    NativeWindow window_;
    NativeBattlefieldRenderer renderer_;
    StrategicCamera camera_;
    ConstructionEditorCameraState constructionCamera_{};
    void SyncConstructionCamera();
    ShipyardBuilderSystem builder_;
    StudioDocumentStore documents_;
    StudioNativeCloseGuard closeGuard_;
    bool closeRecoveryPrepared_=false; // verified blueprint recovery only
    bool modelRecoveryPrepared_=false; // verified independent model recovery only
    bool closePromptActive_=false;
    StudioExitOutcome closeOutcome_=StudioExitOutcome::Unconfirmed;
    GalaxySector emptySector_{};
    ShipyardDockPointerSystem dockPointer_;
    bool pendingCatalogPress_=false;
    bool catalogDragging_=false;
    bool pointerTransform_=false;
    StudioAxis gizmoAxis_=StudioAxis::None;
    ShipyardTransformConstraint previousGizmoConstraint_=ShipyardTransformConstraint::Free;
    bool previousGizmoLocal_=false;
    StudioAxisHandle gizmoStartHandle_{};
    bool gizmoDragged_=false;
    float gizmoPixelAccum_=0;
    float gizmoAngleDelta_=0;
    bool suppressClick_=false;
    int pendingCatalogIndex_=-1;
    float catalogPressX_=0.0f, catalogPressY_=0.0f;
};
} // namespace subspace
