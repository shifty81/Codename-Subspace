#include "studio/StudioApplication.h"
#include "studio/StudioSessionPolicy.h"
#include "studio/StudioProjectPaths.h"
#include "studio/StudioDocumentShortcuts.h"
#include "studio/StudioRecoveryPathPolicy.h"
#include "studio/StudioUnsavedWorkPolicy.h"
#include "studio/StudioFileDialog.h"
#include "studio/StudioClosePolicy.h"
#include "studio/StudioGizmoOverlay.h"
#include "studio/StudioGizmoMath.h"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include "ship_editor/ShipyardDocumentStartupSystem.h"
#include "ship_editor/ShipyardBuildSafetySystem.h"
#include "editor/EditorTransformSpaceSystem.h"
#include "studio/StudioCameraNavigation.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <cmath>

namespace subspace {
StudioApplication::StudioApplication():window_(input_) {}

void StudioApplication::SyncConstructionCamera(){
    // One perspective authority for rendering, picking, projected gizmos and
    // drag placement. The ship transform is NEVER changed by camera movement.
    camera_.SetEditorView(constructionCamera_.eye,
        ConstructionEditorCameraSystem::Target(constructionCamera_),
        constructionCamera_.rollDegrees);
    // Legacy gizmo sensitivity still reads StrategicCamera zoom. Derive that
    // scalar from actual eye/pivot distance; never maintain a second orbit.
    camera_.SetZoom(44.8f/std::max(.35f,constructionCamera_.orbitDistance));
}


int StudioApplication::Run(const std::filesystem::path& openFile,std::uint64_t maxFrames){
    NativeWindowConfig config;
    config.title="Subspace Studio - Ship Authoring";
    config.width=1600;config.height=900;
    if(!window_.Initialize(config)){std::cerr<<"Studio window initialization failed\n";return 2;}
    if(!renderer_.Initialize()){
        std::cerr<<"Studio renderer initialization failed\n";
        window_.Shutdown();return 3;
    }
    // The editor creates a blank authored document. It does NOT start a game
    // frontend, generate a sector, create a player or activate live refitting.
    builder_.Initialize(renderer_.ShipyardCatalog(),ShipyardDocumentStartupSystem::EmptyDocument());
    builder_.SetLiveApplyEnabled(false,true);
    if(!builder_.IsInitialized()){
        std::cerr<<"Studio: certified module catalog unavailable; editing disabled\n";
        renderer_.Shutdown();window_.Shutdown();return 4;
    }
    LoadAuthoringOverrides();
    if(!openFile.empty()){
        std::string error;
        if(!documents_.Open(openFile,builder_,error)){
            std::cerr<<"Studio open failed: "<<error<<'\n';
            renderer_.Shutdown();window_.Shutdown();return 5;
        }
    }
    camera_.SetZoomLimits(.12f,96.0f);
    camera_.SetZoom(1.0f);camera_.SetTargetZoom(1.0f);
    camera_.SetVisualTilt(.46f);camera_.SetVisualHeight(.68f);
    // Start with the existing 3D construction camera. Legacy OrbitVisual
    // clamps elevation to an above-ship tilt and cannot inspect undersides.
    ConstructionEditorCameraSystem::Reset(constructionCamera_,{},16.0f);
    // Match Studio's prior near-55-degree opening angle while changing
    // the underlying navigation to a genuinely three-dimensional orbit.
    ConstructionEditorCameraSystem::Orbit(constructionCamera_,-18.0f,27.0f);
    SyncConstructionCamera();
    // Studio navigation profile: RMB drag orbits; MMB drag pans. Unlike the
    // legacy MMB-orbit editor profile, the native right-drag accumulator now
    // advances, so a real drag cannot be classified as a short RMB click.
    // The game uses its own NativeWindow configuration and is unchanged.
    window_.SetEditorNavigationMode(false);
    std::string closeError;
    if(!closeGuard_.Install(config.title,[this]{return ConfirmClose();},closeError)){
        std::cerr<<"Studio cannot protect window-close data: "<<closeError<<'\n';
        renderer_.Shutdown();window_.Shutdown();return 8;
    }
    std::cout<<"Studio documents: Ctrl+O Open, Ctrl+N New, Ctrl+S Save, Ctrl+Shift+S Save As\n";
    const auto start=std::chrono::steady_clock::now();
    std::uint64_t frames=0;
    while(window_.PumpEvents() && (maxFrames==0 || frames<maxFrames)){
        HandleInput();
        camera_.Update(1.0f/60.0f);
        const float elapsed=std::chrono::duration<float>(std::chrono::steady_clock::now()-start).count();
        window_.BeginFrame(.003f,.007f,.012f,1.0f);
        RenderFrame(elapsed);
        window_.EndFrame();
        input_.EndFrame();
        ++frames;
    }
    int exitCode=0;
    const auto& closing=builder_.Model();
    const StudioUnsavedWorkState unsaved{closing.dirty,closing.socketOverridesDirty,
        closing.definitionOverridesDirty,!closing.modeling.recipe.primitives.empty(),
        closing.interiorStructure.dirty};
    // The WM_CLOSE handler has already written and verified recovery before
    // releasing the window. Do not create a duplicate or erase the receipt.
    if(!closeRecoveryPrepared_ && StudioRecoveryPathPolicy::NeedsRecovery(unsaved.blueprint)){
        std::filesystem::path recovered;
        std::string error;
        if(documents_.SaveExitRecovery(builder_,recovered,error)){
            std::cerr<<"Studio exit: blueprint-only recovery written to "<<recovered.string()<<'\n';
        }else{
            std::cerr<<"Studio exit: unsaved changes; recovery unavailable: "<<error<<'\n';
            exitCode=6; // tooling must not report an unrecoverable close as clean
        }
    }
    if(StudioUnsavedWorkPolicy::HasUnsupportedRecovery(unsaved)){
        std::cerr<<"Studio exit WARNING: socket/definition overrides and editable model/interior drafts are NOT in blueprint recovery\n";
        exitCode=7;
    }
    closeGuard_.Detach();
    renderer_.Shutdown();window_.Shutdown();return exitCode;
}

StudioCloseState StudioApplication::CloseState() const {
    const auto& state=builder_.Model();
    return {state.dirty,state.socketOverridesDirty,state.definitionOverridesDirty,
            !state.modeling.recipe.primitives.empty(),state.interiorStructure.dirty};
}

bool StudioApplication::ConfirmClose(){
    const auto before=CloseState();
    if(!StudioClosePolicy::NeedsPrompt(before))return true;
    if(closePromptActive_)return false; // modal OS message loops may re-enter WM_CLOSE
    closePromptActive_=true;
    struct ResetPrompt { bool& active; ~ResetPrompt(){active=false;} } reset{closePromptActive_};
#ifdef _WIN32
    const int choice=MessageBoxW(GetActiveWindow(),
        L"This Studio session has unsaved work.\n\n"
        L"YES: Save the ship blueprint and close (only if all work is saved).\n"
        L"NO: Close with a separate blueprint recovery when a ship exists.\n"
        L"CANCEL: Keep Studio open.\n\n"
        L"Model/interior drafts and socket/definition edits are not stored in blueprint recovery.",
        L"Subspace Studio - Unsaved Work",MB_YESNOCANCEL|MB_ICONWARNING|MB_DEFBUTTON3);
    if(choice==IDCANCEL||choice==0)return false;
    if(choice==IDYES){
        if(StudioClosePolicy::NeedsExplicitDataLossWarning(before)){
            StudioFileDialog::ShowError("Blueprint Save cannot preserve model/interior drafts or unpublished overrides. Model/interior persistence is not yet implemented: Cancel keeps Studio open; Close with recovery requires a separate explicit loss acknowledgement.");
            return false;
        }
        SaveDocument(); // a canceled chooser or failed write leaves dirty state intact
        if(!StudioClosePolicy::MayCloseAfterSave(CloseState())){
            StudioFileDialog::ShowError("Studio remains open: the document has unsaved changes.");
            return false;
        }
        return true;
    }
    // This is a SECOND, explicit acknowledgement when there is authoring data
    // the blueprint codec cannot recover. Cancel never dismisses the window.
    bool acknowledged=false;
    if(StudioClosePolicy::NeedsExplicitDataLossWarning(before)){
        const int confirm=MessageBoxW(GetActiveWindow(),
            L"WARNING: The blueprint recovery file cannot save editable model or interior drafts,"
            L" or unpublished socket/definition overrides. Those changes may be LOST.\n\n"
            L"Continue closing with only the blueprint recovery?",
            L"Subspace Studio - Partial Recovery",MB_OKCANCEL|MB_ICONSTOP|MB_DEFBUTTON2);
        if(confirm!=IDOK)return false;
        acknowledged=true;
    }
    bool recovered=!before.blueprintDirty;
    if(before.blueprintDirty){
        std::filesystem::path path;
        std::string error;
        recovered=documents_.SaveExitRecovery(builder_,path,error);
        if(!recovered){
            StudioFileDialog::ShowError("Close canceled: verified blueprint recovery failed. "+error);
            return false;
        }
        closeRecoveryPrepared_=true;
        std::cerr<<"Studio close: verified blueprint-only recovery at "<<path.string()<<'\n';
    }
    return StudioClosePolicy::MayCloseWithRecovery(before,recovered,acknowledged);
#else
    return false; // no unguarded close on a backend without a confirmation UI
#endif
}

void StudioApplication::RenderFrame(float elapsed){
    NativeBattlefieldFrame frame{};
    frame.sector=&emptySector_;
    frame.camera=&camera_;
    frame.input=&input_;
    frame.frontendScreen=FrontendScreen::InGame;
    frame.workspaceMode=SandboxWorkspaceMode::ShipBuilder;
    frame.standaloneShipyard=true;
    frame.shipBuilder=&builder_.Model();
    frame.shipBuilderRecipe=&builder_.Recipe();
    frame.shipBuilderAppearance=&builder_.Appearance();
    frame.viewportWidth=window_.GetWidth();frame.viewportHeight=window_.GetHeight();
    frame.pointerX=window_.GetPointerX();frame.pointerY=window_.GetPointerY();
    frame.elapsedSeconds=elapsed;
    renderer_.Render(frame);
    // Overlay uses the exact same projected ship position, viewport dock bounds,
    // and input picking snapshot. It never owns the blueprint or renderer.
    auto gizmo=StudioAxisGizmo::Build(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());
    for(auto& handle:gizmo.handles){
        if(!handle.valid)continue;
        const auto& dock=builder_.Model().dockWorkspace;
        const int h=std::max(1,static_cast<int>(ShipyardBuilderSystem::Layout(
            builder_.Model(),window_.GetWidth(),window_.GetHeight()).statusY));
        const float top=gizmo.viewportTop;
        if(ShipyardDockPointerSystem::CoversFloatingPanel(dock,window_.GetWidth(),h,top,
                    handle.center.x,handle.center.y)||
           ShipyardDockPointerSystem::CoversFloatingPanel(dock,window_.GetWidth(),h,top,
                    handle.tip.x,handle.tip.y))handle.valid=false;
    }
    gizmo.visible=false;for(const auto& handle:gizmo.handles)gizmo.visible|=handle.valid;
    // The measurement HUD cannot paint across floating docks. The normal
    // viewport scissor is insufficient because floating panels live inside
    // the viewport rectangle and are composed before this OpenGL overlay.
    if(gizmo.readoutVisible){
        const auto& dock=builder_.Model().dockWorkspace;
        const auto layout=ShipyardBuilderSystem::Layout(builder_.Model(),window_.GetWidth(),window_.GetHeight());
        const int dockHeight=std::max(1,static_cast<int>(layout.statusY));
        for(int row=0;row<3&&gizmo.readoutVisible;++row){
            for(int col=0;col<3&&gizmo.readoutVisible;++col){
                const float x=gizmo.viewportLeft+9.0f+217.5f*col;
                const float y=gizmo.viewportTop+9.0f+53.0f*row;
                if(ShipyardDockPointerSystem::CoversFloatingPanel(dock,window_.GetWidth(),dockHeight,
                    gizmo.viewportTop,x,y))gizmo.readoutVisible=false;
            }
        }
    }
    const auto hovered=gizmo.Pick(window_.GetPointerX(),window_.GetPointerY());
    StudioGizmoOverlay::Draw(gizmo,window_.GetWidth(),window_.GetHeight(),
        gizmoAxis_,hovered,builder_.Model().transformTool==ShipyardTransformTool::Rotate,
        gizmoAngleDelta_);
}

void StudioApplication::LoadAuthoringOverrides(){
    const auto sockets=StudioProjectPaths::SocketOverrides();
    const auto definitions=StudioProjectPaths::DefinitionOverrides();
    if(sockets.empty()||definitions.empty()){
        std::cerr<<"Studio: repository root not found; authoring overrides unavailable\n";return;
    }
    std::error_code ec;
    std::string error;
    std::size_t count=0;
    if(std::filesystem::exists(sockets,ec)){
        if(builder_.LoadSocketOverrides(sockets.string(),&error,&count)){
            renderer_.ApplyShipyardCatalogAuthoringOverrides(builder_.Model().catalog);
            std::cout<<"Studio: loaded socket overrides for "<<count<<" modules\n";
        }else std::cerr<<"Studio socket override load blocked: "<<error<<'\n';
    }
    ec.clear();error.clear();count=0;
    if(std::filesystem::exists(definitions,ec)){
        if(builder_.LoadDefinitionOverrides(definitions.string(),&error,&count)){
            renderer_.ApplyShipyardCatalogAuthoringOverrides(builder_.Model().catalog);
            std::cout<<"Studio: loaded definition overrides for "<<count<<" modules\n";
        }else std::cerr<<"Studio definition override load blocked: "<<error<<'\n';
    }
}
void StudioApplication::SaveAuthoringOverrides(bool sockets){
    const auto path=sockets?StudioProjectPaths::SocketOverrides():StudioProjectPaths::DefinitionOverrides();
    if(path.empty()){std::cerr<<"Studio overrides: repository root not found\n";return;}
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(),ec);
    if(ec){std::cerr<<"Studio overrides: cannot create authoring directory: "<<ec.message()<<'\n';return;}
    std::string error;std::size_t changed=0;
    const bool ok=sockets?builder_.SaveSocketOverrides(path.string(),&error,&changed):
                         builder_.SaveDefinitionOverrides(path.string(),&error,&changed);
    if(!ok){std::cerr<<"Studio overrides SAVE failed: "<<error<<'\n';return;}
    renderer_.ApplyShipyardCatalogAuthoringOverrides(builder_.Model().catalog);
    if(sockets)builder_.MarkSocketOverridesSaved(path.filename().string());
    else builder_.MarkDefinitionOverridesSaved(path.filename().string());
    std::cout<<"Studio overrides saved: "<<changed<<" module(s)\n";
}

void StudioApplication::ShowDocumentError(const std::string& error){
    builder_.MarkSaved("ERROR "+error); // error status must not mark unsaved work clean
    StudioFileDialog::ShowError(error);
}
void StudioApplication::SaveDocument(){
    // First save chooses an explicit filename. Subsequent saves retain their
    // authoritative document path and the transactional backup protocol.
    if(documents_.Path().empty()){SaveAsDocument();return;}
    std::string error;
    if(!documents_.Save(builder_,error))ShowDocumentError("Save blocked: "+error);
    else {
        std::cout<<"Studio saved "<<documents_.Path().string()<<'\n';
        if(!error.empty())StudioFileDialog::ShowError(error); // preserved recovery backup
    }
}
void StudioApplication::SaveAsDocument(){
    if(builder_.Recipe().modules.empty()){
        ShowDocumentError("Add a ship module before saving a blueprint; model-only documents are not yet serializable");
        return;
    }
    auto directory=StudioProjectPaths::Blueprints();
    if(directory.empty()){
        ShowDocumentError("Project root unavailable: cannot choose a safe blueprint directory");return;
    }
    std::error_code ec;
    std::filesystem::create_directories(directory,ec);
    if(ec){ShowDocumentError("Cannot prepare blueprint directory: "+ec.message());return;}
    std::filesystem::path selected;
    std::string error;
    if(!StudioFileDialog::ChooseSaveAs(directory,documents_.Path(),selected,error)){
        if(!error.empty())ShowDocumentError(error);
        return;
    }
    if(!documents_.SaveAs(selected,builder_,error))ShowDocumentError("Save As blocked: "+error);
    else {
        std::cout<<"Studio saved as "<<documents_.Path().string()<<'\n';
        if(!error.empty())StudioFileDialog::ShowError(error);
    }
}
void StudioApplication::OpenDocument(){
    // Reject before opening the native picker; no document may silently
    // replace pending edits or an unsaved authoring draft.
    const auto& state=builder_.Model();
    if(StudioUnsavedWorkPolicy::HasUnsaved({state.dirty,state.socketOverridesDirty,
            state.definitionOverridesDirty,!state.modeling.recipe.primitives.empty(),
            state.interiorStructure.dirty})){
        ShowDocumentError("Unsaved blueprint, socket, definition, model or interior edits: save before Open");return;
    }
    const auto directory=StudioProjectPaths::Blueprints();
    if(directory.empty()){ShowDocumentError("Project root unavailable: cannot locate blueprints");return;}
    std::filesystem::path selected;
    std::string error;
    if(!StudioFileDialog::ChooseOpen(directory,selected,error)){
        if(!error.empty())ShowDocumentError(error);
        return;
    }
    if(!documents_.Open(selected,builder_,error)){ShowDocumentError("Open blocked: "+error);return;}
    pendingCatalogPress_=false;catalogDragging_=false;pointerTransform_=false;
    gizmoAxis_=StudioAxis::None;gizmoDragged_=false;gizmoPixelAccum_=0;gizmoAngleDelta_=0;
    dockPointer_.Cancel();suppressClick_=true;
    std::cout<<"Studio opened "<<documents_.Path().string()<<'\n';
}
void StudioApplication::NewDocument(){
    std::string error;
    if(!documents_.New(builder_,error)){ShowDocumentError("New blocked: "+error);return;}
    pendingCatalogPress_=false;catalogDragging_=false;pointerTransform_=false;
    gizmoAxis_=StudioAxis::None;gizmoDragged_=false;gizmoPixelAccum_=0;gizmoAngleDelta_=0;
    dockPointer_.Cancel();suppressClick_=true;
    std::cout<<"Studio: new empty document\n";
}

void StudioApplication::RouteControl(ShipyardBuilderCommand command,int value){
    if(command==ShipyardBuilderCommand::SaveBlueprint){SaveDocument();return;}
    if(command==ShipyardBuilderCommand::NewEmptyDocument){NewDocument();return;}
    // No game menu, warp, engine pause or player refit is routed from Studio.
    if(command==ShipyardBuilderCommand::Apply)return;
    builder_.Activate(command,value);
    if(builder_.ConsumeSaveRequested())SaveDocument();
    if(builder_.ConsumeSocketOverridesSaveRequested())SaveAuthoringOverrides(true);
    if(builder_.ConsumeDefinitionOverridesSaveRequested())SaveAuthoringOverrides(false);
}

void StudioApplication::RestoreGizmoConstraint(){
    if(previousGizmoConstraint_==ShipyardTransformConstraint::Free)
        builder_.ClearTransformConstraint();
    else {
        builder_.SetTransformConstraint(previousGizmoConstraint_,false);
        if(previousGizmoLocal_)builder_.SetTransformConstraint(previousGizmoConstraint_,true);
    }
}
void StudioApplication::HandleEscape(){
    if(gizmoAxis_!=StudioAxis::None){
        builder_.CancelTransform();RestoreGizmoConstraint();
        gizmoAxis_=StudioAxis::None;gizmoDragged_=false;gizmoPixelAccum_=0;
        gizmoAngleDelta_=0;pointerTransform_=false;suppressClick_=true;
        return;
    }
    pendingCatalogPress_=false;catalogDragging_=false;pointerTransform_=false;
    gizmoAxis_=StudioAxis::None;gizmoDragged_=false;gizmoPixelAccum_=0;gizmoAngleDelta_=0;
    dockPointer_.Cancel();suppressClick_=true;
    if(builder_.Model().assetSearchFocused){builder_.BlurAssetSearch();return;}
    // Socket manipulation has a distinct transaction from module transforms.
    if(builder_.CancelSocketTransform())return;
    const auto& m=builder_.Model();
    const StudioEscapeState state{m.dragPreview.active||m.dragPreview.staged,
        m.transform.active,m.dcc.commandPaletteOpen,m.openMenu>=0,
        m.transformConstraint!=ShipyardTransformConstraint::Free};
    switch(StudioSessionPolicy::Escape(state)){
    case StudioEscapeAction::CancelPlacement:builder_.CancelCatalogDrag();break;
    case StudioEscapeAction::CancelTransform:builder_.CancelTransform();break;
    case StudioEscapeAction::CloseCommandPalette:RouteControl(ShipyardBuilderCommand::DccToggleCommandPalette);break;
    case StudioEscapeAction::CloseMenu:
        // Existing UI menu commands toggle the active menu when clicked again.
        RouteControl(static_cast<ShipyardBuilderCommand>(static_cast<int>(ShipyardBuilderCommand::MenuFile)+m.openMenu));break;
    case StudioEscapeAction::ClearConstraint:builder_.ClearTransformConstraint();break;
    case StudioEscapeAction::NoOp:break;
    }
}
void StudioApplication::HandleInput(){
    if(input_.WasPressed(InputAction::MenuBack))HandleEscape();
    const auto text=window_.ConsumeTextInput();
    if(builder_.Model().assetSearchFocused&&!text.empty())builder_.HandleAssetSearchInput(text);
    if(input_.WasPressed(InputAction::Undo))builder_.UndoAuthoring();
    if(input_.WasPressed(InputAction::Redo))builder_.RedoAuthoring();
    // NativeWindow's legacy input vocabulary maps O/N/S as ordinary actions.
    // Studio alone interprets their Ctrl variants: game input remains unchanged.
    const bool scalePressed=input_.WasPressed(InputAction::EditorToolScale);
    const auto documentShortcut=StudioDocumentShortcuts::Resolve(
        window_.IsControlDown(),window_.IsShiftDown(),
        input_.WasPressed(InputAction::OpenExploration),
        input_.WasPressed(InputAction::OpenSystemMap),scalePressed);
    switch(documentShortcut){
    case StudioDocumentShortcut::Open:OpenDocument();break;
    case StudioDocumentShortcut::New:NewDocument();break;
    case StudioDocumentShortcut::Save:SaveDocument();break;
    case StudioDocumentShortcut::SaveAs:SaveAsDocument();break;
    case StudioDocumentShortcut::None:break;
    }
    if(input_.WasPressed(InputAction::SaveBlueprint)&&
       documentShortcut!=StudioDocumentShortcut::Save&&
       documentShortcut!=StudioDocumentShortcut::SaveAs)SaveDocument();
    if(input_.WasPressed(InputAction::MenuAccept))RouteControl(ShipyardBuilderCommand::AddModule);
    if(input_.WasPressed(InputAction::EditorFrameShip)){
        // F reframes the whole assembly without resetting inspection orientation.
        // Framing individual components needs a separate precise bounds pass.
        float radius=6.0f;
        for(const auto& part:builder_.Recipe().modules){
            radius=std::max(radius,std::sqrt(part.x*part.x+part.y*part.y+part.z*part.z)*.24f+3.0f);
        }
        ConstructionEditorCameraSystem::FramePreservingOrientation(constructionCamera_,{},radius);
        SyncConstructionCamera();
    }
    if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);
    if(input_.WasPressed(InputAction::EditorToolMove))RouteControl(ShipyardBuilderCommand::ToolMove);
    if(input_.WasPressed(InputAction::EditorToolRotate))RouteControl(ShipyardBuilderCommand::ToolRotate);
    if(scalePressed&&!window_.IsControlDown())RouteControl(ShipyardBuilderCommand::ToolScale);
    if(input_.WasPressed(InputAction::DccCommandSearch))RouteControl(ShipyardBuilderCommand::DccToggleCommandPalette);
    if(input_.WasPressed(InputAction::DccWorkspaceNext))RouteControl(ShipyardBuilderCommand::DccWorkspaceNext);
    if(input_.WasPressed(InputAction::DccWorkspacePrevious))RouteControl(ShipyardBuilderCommand::DccWorkspacePrevious);
    float pressX=0,pressY=0;
    if(window_.ConsumePrimaryPress(pressX,pressY)){
        const auto layout=ShipyardBuilderSystem::Layout(builder_.Model(),window_.GetWidth(),window_.GetHeight());
        const bool dockCaptured=dockPointer_.Begin(builder_.Model().dockWorkspace,
            window_.GetWidth(),std::max(1,static_cast<int>(layout.statusY)),layout.viewportTop,pressX,pressY);
        if(!dockCaptured){
            const auto hit=ShipyardBuilderSystem::HitTest(builder_.Model(),window_.GetWidth(),window_.GetHeight(),pressX,pressY);
            if(hit.command==ShipyardBuilderCommand::SelectModule){
                RouteControl(ShipyardBuilderCommand::SelectModule,hit.value);
                pendingCatalogPress_=true;pendingCatalogIndex_=hit.value;
                catalogPressX_=pressX;catalogPressY_=pressY;
            }else if(hit.command==ShipyardBuilderCommand::None &&
                !ShipyardDockPointerSystem::CoversFloatingPanel(builder_.Model().dockWorkspace,
                   window_.GetWidth(),std::max(1,static_cast<int>(layout.statusY)),
                   layout.viewportTop,pressX,pressY)){
                const auto snapshot=StudioAxisGizmo::Build(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());
                const auto axis=snapshot.Pick(pressX,pressY);
                if(axis!=StudioAxis::None){
                    const auto* handle=snapshot.Handle(axis);
                    const bool obscured=!handle||ShipyardDockPointerSystem::CoversFloatingPanel(
                        builder_.Model().dockWorkspace,window_.GetWidth(),
                        std::max(1,static_cast<int>(layout.statusY)),layout.viewportTop,
                        handle->tip.x,handle->tip.y);
                    if(!obscured&&builder_.BeginSelectedTransform()){
                        previousGizmoConstraint_=builder_.Model().transformConstraint;
                        previousGizmoLocal_=builder_.Model().transformConstraintLocal;
                        gizmoAxis_=axis;gizmoStartHandle_=*handle;
                        gizmoPixelAccum_=0;gizmoAngleDelta_=0;gizmoDragged_=false;
                        // Rotation's packed delta order is pitch, yaw, roll;
                        // the physical gizmo's order is X, Y, Z. Apply the
                        // mapped FIELD constraint, not the visible axis index.
                        const auto constrained=builder_.Model().transformTool==ShipyardTransformTool::Rotate?
                            StudioGizmoMath::RotationFieldAxis(axis):axis;
                        builder_.SetTransformConstraint(constrained==StudioAxis::X?ShipyardTransformConstraint::X:
                            constrained==StudioAxis::Y?ShipyardTransformConstraint::Y:ShipyardTransformConstraint::Z,false);
                        pointerTransform_=true;suppressClick_=true;
                    }
                }else{
                const int picked=NativeBattlefieldRenderer::PickShipyardModule(builder_.Model().catalog,
                    builder_.Recipe(),camera_,window_.GetWidth(),window_.GetHeight(),
                    pressX,pressY,0,0,0,.24f,.22f,true);
                if(picked>=0){
                    RouteControl(ShipyardBuilderCommand::SelectPlaced,picked);
                    if(builder_.Model().transformTool!=ShipyardTransformTool::Select){
                        pointerTransform_=builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets?
                            builder_.BeginSelectedSocketTransform():builder_.BeginSelectedTransform();
                    }
                }
                } // no handle: ordinary part selection remains available
            }
        }
    }
    float dragX=0,dragY=0;
    if(window_.ConsumePrimaryDragDelta(dragX,dragY)){
        if(dockPointer_.Active()){
            const auto layout=ShipyardBuilderSystem::Layout(builder_.Model(),window_.GetWidth(),window_.GetHeight());
            dockPointer_.Drag(builder_.MutableDockWorkspace(),dragX,dragY,
                window_.GetWidth(),std::max(1,static_cast<int>(layout.statusY)),layout.viewportTop);
        }else{
            if(pendingCatalogPress_&&!catalogDragging_){
                const auto dx=window_.GetPointerX()-catalogPressX_;
                const auto dy=window_.GetPointerY()-catalogPressY_;
                if(dx*dx+dy*dy>=64.0f){
                    catalogDragging_=builder_.BeginCatalogDrag(pendingCatalogIndex_);
                    pendingCatalogPress_=false;
                }
            }
            if(catalogDragging_){
                const auto world=NativeBattlefieldRenderer::ScreenToWorld(window_.GetPointerX(),window_.GetPointerY(),
                    window_.GetWidth(),window_.GetHeight(),camera_);
                const float yaw=EditorTransformSpaceSystem::RenderedRootYawRadians(0.0f,builder_.Recipe().forwardVisualYawDegrees);
                builder_.UpdateCatalogDrag(EditorTransformSpaceSystem::WorldToAssemblyDelta(world,yaw));
            }else if(pointerTransform_){
                if(gizmoAxis_!=StudioAxis::None){
                    // One pointer gesture = one authoritative ship-axis transaction.
                    // Absolute targets avoid losing sub-snap drag deltas between frames.
                    const auto& tx=builder_.Model().transform;
                    const auto axis=gizmoAxis_;
                    const bool rotate=builder_.Model().transformTool==ShipyardTransformTool::Rotate;
                    gizmoPixelAccum_+=StudioGizmoMath::DragScalar(gizmoStartHandle_,dragX,dragY,rotate);
                    if(std::fabs(gizmoPixelAccum_)>0.001f)gizmoDragged_=true;
                    const bool fine=window_.IsShiftDown();
                    const float step=std::max(.01f,camera_.GetZoom());
                    if(rotate){
                        const auto before=StudioGizmoMath::RotationComponent(axis,tx.before.pitchDegrees,
                            tx.before.yawDegrees,tx.before.rollDegrees);
                        const auto working=StudioGizmoMath::RotationComponent(axis,tx.working.pitchDegrees,
                            tx.working.yawDegrees,tx.working.rollDegrees);
                        float desired=before+gizmoPixelAccum_*(fine?.035f:.35f);
                        if(tx.snap&&!fine&&tx.rotationSnapDegrees>0)
                            desired=std::round(desired/tx.rotationSnapDegrees)*tx.rotationSnapDegrees;
                        const float delta=desired-working;
                        const float sourceDelta=fine?delta*10.0f:delta;
                        const Vector3 rotation{axis==StudioAxis::X?sourceDelta:0,
                            axis==StudioAxis::Z?sourceDelta:0,axis==StudioAxis::Y?sourceDelta:0};
                        if(std::fabs(delta)>1e-5f)builder_.RotateSelected(rotation,fine);
                        const auto& applied=builder_.Model().transform;
                        gizmoAngleDelta_=StudioGizmoMath::RotationComponent(axis,applied.working.pitchDegrees,
                            applied.working.yawDegrees,applied.working.rollDegrees)-before;
                    }else if(builder_.Model().transformTool==ShipyardTransformTool::Move){
                        const float before=StudioGizmoMath::Component(axis,tx.before.x,tx.before.y,tx.before.z);
                        const float working=StudioGizmoMath::Component(axis,tx.working.x,tx.working.y,tx.working.z);
                        const float desired=before+gizmoPixelAccum_*.012f/std::max(.35f,step)*(fine?.1f:1.0f);
                        const float delta=desired-working;
                        const float sourceDelta=fine?delta*10.0f:delta;
                        const Vector3 translation{axis==StudioAxis::X?sourceDelta:0,
                            axis==StudioAxis::Y?sourceDelta:0,axis==StudioAxis::Z?sourceDelta:0};
                        if(std::fabs(delta)>1e-6f)builder_.TranslateSelected(translation,fine);
                    }else if(builder_.Model().transformTool==ShipyardTransformTool::Scale){
                        const float before=StudioGizmoMath::Component(axis,tx.before.scaleX,tx.before.scaleY,tx.before.scaleZ);
                        const float working=StudioGizmoMath::Component(axis,tx.working.scaleX,tx.working.scaleY,tx.working.scaleZ);
                        const float desired=std::clamp(before+gizmoPixelAccum_*.003f*(fine?.1f:1.0f),.10f,4.0f);
                        const float delta=desired-working,sourceDelta=fine?delta*10.0f:delta;
                        const Vector3 scale{axis==StudioAxis::X?sourceDelta:0,
                            axis==StudioAxis::Y?sourceDelta:0,axis==StudioAxis::Z?sourceDelta:0};
                        if(std::fabs(delta)>1e-6f)builder_.ScaleSelected(scale,fine);
                    }
                }else{
                const bool fine=window_.IsShiftDown();
                const bool socket=builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets;
                const auto tool=builder_.Model().transformTool;
                if(tool==ShipyardTransformTool::Move){
                    const float scale=.012f/std::max(.35f,camera_.GetZoom());
                    const Vector3 world=camera_.ViewRightPlanar()*(dragX*scale)+camera_.ViewUpPlanar()*(-dragY*scale);
                    const float yaw=EditorTransformSpaceSystem::RenderedRootYawRadians(0.0f,builder_.Recipe().forwardVisualYawDegrees);
                    auto delta=EditorTransformSpaceSystem::WorldToAssemblyDelta(world,yaw);
                    if(socket){if(const auto* part=builder_.SelectedPlacedModule())delta=EditorTransformSpaceSystem::AssemblyToModuleLocalDelta(delta,*part);builder_.TranslateSelectedSocket(delta,fine);}
                    else builder_.TranslateSelected(delta,fine);
                }else if(tool==ShipyardTransformTool::Rotate){
                    const Vector3 rotation{-dragY*.35f,dragX*.35f,0};
                    if(socket)builder_.RotateSelectedSocket(rotation,fine);else builder_.RotateSelected(rotation,fine);
                }else if(tool==ShipyardTransformTool::Scale&&!socket){
                    const float amount=(dragX-dragY)*.003f;
                    builder_.ScaleSelected({amount,amount,amount},fine);
                }
                } // free manipulation, independent from axis gizmo
            }
        }
    }
    float releaseX=0,releaseY=0;
    if(window_.ConsumePrimaryRelease(releaseX,releaseY)){
        if(dockPointer_.Active()){
            const auto layout=ShipyardBuilderSystem::Layout(builder_.Model(),window_.GetWidth(),window_.GetHeight());
            suppressClick_=dockPointer_.End(builder_.MutableDockWorkspace(),releaseX,releaseY,
                window_.GetWidth(),std::max(1,static_cast<int>(layout.statusY)),layout.viewportTop);
        }
        if(catalogDragging_)builder_.StageCatalogDrag();
        catalogDragging_=false;pendingCatalogPress_=false;pendingCatalogIndex_=-1;
        if(pointerTransform_){
            if(gizmoAxis_!=StudioAxis::None){
                const auto& tx=builder_.Model().transform;
                const auto& a=tx.before;const auto& b=tx.working;
                const auto changed=[](float x,float y){return std::fabs(x-y)>1e-5f;};
                const bool mutated=changed(a.x,b.x)||changed(a.y,b.y)||changed(a.z,b.z)||
                    changed(a.pitchDegrees,b.pitchDegrees)||changed(a.yawDegrees,b.yawDegrees)||
                    changed(a.rollDegrees,b.rollDegrees)||changed(a.scaleX,b.scaleX)||
                    changed(a.scaleY,b.scaleY)||changed(a.scaleZ,b.scaleZ);
                if(gizmoDragged_&&mutated)builder_.CommitTransform();
                else builder_.CancelTransform(); // click/sub-snap motion is not undo history
                RestoreGizmoConstraint();
                gizmoAxis_=StudioAxis::None;gizmoDragged_=false;
                gizmoPixelAccum_=0;gizmoAngleDelta_=0;
            }else if(builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets)
                builder_.CommitSocketTransform();
            else builder_.CommitTransform();
            pointerTransform_=false;suppressClick_=true;
        }
    }
    float clickX=0,clickY=0;
    if(window_.ConsumePrimaryClick(clickX,clickY)){
        if(!suppressClick_){
            const auto hit=ShipyardBuilderSystem::HitTest(builder_.Model(),window_.GetWidth(),window_.GetHeight(),clickX,clickY);
            if(hit.enabled&&hit.command!=ShipyardBuilderCommand::None)RouteControl(hit.command,hit.value);
        }
        suppressClick_=false;
    }
    const float wheel=window_.ConsumeWheelDelta();
    if(wheel!=0.0f){
        if(!builder_.HandleWheel(window_.GetPointerX(),window_.GetPointerY(),wheel,window_.GetWidth(),window_.GetHeight()))
            ConstructionEditorCameraSystem::Dolly(constructionCamera_,wheel);
    }
    float dx=0,dy=0;
    if(window_.ConsumeCameraOrbitDelta(dx,dy)){
        // RMB: horizontal yaw, vertical pitch, including underneath the ship.
        // Shift+RMB: roll about the viewing axis while preserving orbit pivot.
        StudioCameraNavigation::Orbit(constructionCamera_,dx,dy,window_.IsShiftDown());
    }
    if(window_.ConsumeCameraPanDelta(dx,dy))
        ConstructionEditorCameraSystem::PanPixels(constructionCamera_,dx,dy,
            static_cast<float>(std::max(1,window_.GetHeight())));
    SyncConstructionCamera();
    ShipyardBuildSafetySystem::SuppressFlightAndWeapons(input_);
}
} // namespace subspace
