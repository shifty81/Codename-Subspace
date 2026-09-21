#include "studio/StudioApplication.h"
#include "studio/StudioSessionPolicy.h"
#include "studio/StudioProjectPaths.h"
#include "studio/StudioDocumentShortcuts.h"
#include "studio/StudioRecoveryPathPolicy.h"
#include "studio/StudioUnsavedWorkPolicy.h"
#include "studio/StudioFileDialog.h"
#include "studio/StudioClosePolicy.h"
#include "studio/StudioExitOutcomePolicy.h"
#include "studio/StudioGizmoOverlay.h"
#include "studio/StudioOverlayProgramScope.h"
#include "studio/StudioGizmoMath.h"
#include "studio/StudioGizmoProjectionPolicy.h"
#include "studio/StudioInteriorPreviewKey.h"
#include "studio/StudioToolInteractionPolicy.h"
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
namespace {
// Rendering and pointer-down share one occlusion-filtered snapshot. Hidden
// handles are never pickable through a floating panel.
StudioGizmoSnapshot VisibleGizmoSnapshot(const ShipyardBuilderRuntimeModel& model,
        const StrategicCamera& camera,int width,int height){
    auto snapshot=StudioAxisGizmo::Build(model,camera,width,height);
    if(!snapshot.visible)return snapshot;
    const auto layout=ShipyardBuilderSystem::Layout(model,width,height);
    const int dockHeight=std::max(1,static_cast<int>(layout.statusY));
    // R4: all-angle projection and dock occlusion share one handle layout.
    // The previous sample-and-delete pass discarded axes without retrying.
    // Materialize the existing compositor geometry ONCE. Calling the pointer
    // helper for each of ~300 reflow probes would rebuild the dock tree on
    // every probe, causing severe per-frame allocation and layout churn.
    const auto layers=ShipyardPanelCompositorSystem::Snapshot(
        model.dockWorkspace,width,dockHeight,layout.viewportTop);
    const auto occluded=[&](float x,float y){
        if(ShipyardPanelCompositorSystem::TopFloatingAt(layers,x,y))return true;
        // Preserve the pointer system's overlay-first rule for docked bodies.
        if(model.dockWorkspace.id=="shipyard")for(const auto& layer:layers)
            if(layer.visible&&layer.panelId!="viewport"&&
               ShipyardPanelCompositorSystem::Contains(layer.rect,x,y))return true;
        return false;
    };
    snapshot.handles=StudioGizmoProjectionPolicy::ReflowForOcclusion(
        snapshot.handles,
        {snapshot.viewportLeft+10.0f,snapshot.viewportTop+10.0f,
         snapshot.viewportRight-10.0f,snapshot.viewportBottom-10.0f},occluded);
    snapshot.visible=false;for(const auto& h:snapshot.handles)snapshot.visible|=h.valid;
    return snapshot;
}

int PickModelPrimitive(const ShipyardBuilderRuntimeModel& model,const StrategicCamera& camera,
                       int width,int height,float x,float y){
    if(model.workspaceMode!=ShipyardWorkspaceMode::Model||model.modeling.recipe.primitives.empty())return -1;
    const auto layout=ShipyardBuilderSystem::Layout(model,width,height);
    if(x<layout.viewportLeft||x>layout.viewportRight||y<layout.viewportTop||y>layout.viewportBottom)return -1;
    float best=1.0e30f;int picked=-1;
    for(std::size_t i=0;i<model.modeling.recipe.primitives.size();++i){
        const auto& p=model.modeling.recipe.primitives[i];
        const auto center=NativeBattlefieldRenderer::WorldToScreen(p.position,width,height,camera);
        if(!center.visible)continue;
        float radius=14.0f;
        const Vector3 half=p.size*.5f;
        for(const Vector3 d: {Vector3{half.x,0,0},Vector3{0,half.y,0},Vector3{0,0,half.z}}){
            const auto edge=NativeBattlefieldRenderer::WorldToScreen(p.position+d,width,height,camera);
            if(edge.visible){const float dx=edge.x-center.x,dy=edge.y-center.y;radius=std::max(radius,std::sqrt(dx*dx+dy*dy));}
        }
        radius=std::clamp(radius,14.0f,180.0f);
        const float dx=x-center.x,dy=y-center.y,d2=dx*dx+dy*dy;
        if(d2<=radius*radius&&d2<best){best=d2;picked=static_cast<int>(i);}
    }
    return picked;
}
} // namespace
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
    config.title="Null Harbor Studio - Ship Authoring";
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
    const StudioUnsavedWorkState unsaved{closing.dirty&&!closing.recipe.modules.empty(),closing.socketOverridesDirty,
        closing.definitionOverridesDirty,builder_.HasUnsavedModeling(),
        closing.interiorStructure.dirty};
    // The WM_CLOSE handler has already written and verified recovery before
    // releasing the window. Do not create a duplicate or erase the receipt.
    if(!StudioExitOutcomePolicy::IsUserConfirmed(closeOutcome_) &&
       !closeRecoveryPrepared_ && StudioRecoveryPathPolicy::NeedsRecovery(unsaved.blueprint)){
        std::filesystem::path recovered;
        std::string error;
        if(documents_.SaveExitRecovery(builder_,recovered,error)){
            closeRecoveryPrepared_=true;
            std::cerr<<"Studio exit: blueprint-only recovery written to "<<recovered.string()<<'\n';
        }else{
            std::cerr<<"Studio exit: unsaved changes; recovery unavailable: "<<error<<'\n';
            exitCode=6; // tooling must not report an unrecoverable close as clean
        }
    }
    if(!StudioExitOutcomePolicy::IsUserConfirmed(closeOutcome_) &&
       unsaved.modelDraft && !modelRecoveryPrepared_){
        std::filesystem::path recovered;std::string error;
        if(documents_.SaveModelExitRecovery(builder_,recovered,error)){
            modelRecoveryPrepared_=true;
            std::cerr<<"Studio exit: verified model recovery written to "<<recovered.string()<<'\n';
        }else{
            std::cerr<<"Studio exit: model recovery unavailable: "<<error<<'\n';
            exitCode=6;
        }
    }
    const bool unsupported=StudioUnsavedWorkPolicy::HasUnsupportedRecovery(unsaved,modelRecoveryPrepared_);
    if(unsupported){
        std::cerr<<"Studio exit WARNING: socket/definition overrides, interior, or unrecovered model drafts may be lost\n";
    }
    exitCode=StudioExitOutcomePolicy::ExitCode(closeOutcome_,exitCode==6,unsupported);
    std::cerr<<"STUDIO_EXIT_RESULT schema=subspace.studio-exit.v1 outcome="
             <<StudioExitOutcomePolicy::Name(closeOutcome_)
             <<" blueprint_recovery_verified="<<(closeRecoveryPrepared_?"true":"false")
             <<" model_recovery_verified="<<(modelRecoveryPrepared_?"true":"false")
             <<" unsupported_drafts="<<(unsupported?"true":"false")
             <<" process_exit="<<exitCode<<'\n';
    closeGuard_.Detach();
    renderer_.Shutdown();window_.Shutdown();return exitCode;
}

StudioCloseState StudioApplication::CloseState() const {
    const auto& state=builder_.Model();
    return {state.dirty&&!state.recipe.modules.empty(),state.socketOverridesDirty,state.definitionOverridesDirty,
            builder_.HasUnsavedModeling(),state.interiorStructure.dirty};
}

bool StudioApplication::ConfirmClose(){
    const auto before=CloseState();
    if(!StudioClosePolicy::NeedsPrompt(before)){
        closeOutcome_=StudioExitOutcome::Clean;return true;
    }
    if(closePromptActive_)return false; // modal OS message loops may re-enter WM_CLOSE
    closePromptActive_=true;
    struct ResetPrompt { bool& active; ~ResetPrompt(){active=false;} } reset{closePromptActive_};
#ifdef _WIN32
    const int choice=MessageBoxW(GetActiveWindow(),
        L"This Studio session has unsaved work.\n\n"
        L"YES: Save the active Studio document and close if all work is saved.\n"
        L"NO: Close with verified separate blueprint/model recovery files.\n"
        L"CANCEL: Keep Studio open.\n\n"
        L"Unpublished socket/definition overrides and interior drafts still require explicit save/review.",
        L"Subspace Studio - Unsaved Work",MB_YESNOCANCEL|MB_ICONWARNING|MB_DEFBUTTON3);
    if(choice==IDCANCEL||choice==0)return false;
    if(choice==IDYES){
        if(StudioClosePolicy::NeedsExplicitDataLossWarning(before,true)){
            StudioFileDialog::ShowError("This save cannot preserve unsaved interior or unpublished socket/definition overrides. Cancel keeps Studio open; save those lanes before closing.");
            return false;
        }
        SaveDocument(); // a canceled chooser or failed write leaves dirty state intact
        if(!StudioClosePolicy::MayCloseAfterSave(CloseState())){
            StudioFileDialog::ShowError("Studio remains open: the document has unsaved changes.");
            return false;
        }
        closeOutcome_=StudioExitOutcome::Saved;
        return true;
    }
    // Recover the model independently: blueprint recovery cannot contain it,
    // even when an assembly and model draft coexist in the same session.
    bool modelRecovered=!before.modelDraft;
    if(before.modelDraft){
        std::filesystem::path modelPath;std::string error;
        if(!documents_.SaveModelExitRecovery(builder_,modelPath,error)){
            StudioFileDialog::ShowError("Close canceled: verified model recovery failed. "+error);
            return false;
        }
        modelRecovered=true; // publish receipt only once close is accepted
        std::cerr<<"Studio close: verified independent model recovery at "<<modelPath.string()<<'\n';
    }
    // Only truly unsupported lanes require a SECOND loss acknowledgement.
    // Cancel never dismisses the window.
    bool acknowledged=false;
    if(StudioClosePolicy::NeedsExplicitDataLossWarning(before,modelRecovered)){
        const int confirm=MessageBoxW(GetActiveWindow(),
            L"WARNING: automatic recovery cannot save unsaved interior drafts,"
            L" or unpublished socket/definition overrides. Those changes may be LOST.\n\n"
            L"Continue closing with the available verified recovery files?",
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
    if(!StudioClosePolicy::MayCloseWithRecovery(before,recovered,acknowledged,modelRecovered))return false;
    if(before.modelDraft)modelRecoveryPrepared_=true;
    // The user requested recovery; independent model/blueprint recovery is
    // verified, and any remaining unsupported data loss was explicitly approved.
    // This is normal app termination, NOT proof of a complete Studio save.
    closeOutcome_=StudioExitOutcome::UserConfirmedPartialRecovery;
    return true;
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
    // The game already supplies editorInteriorShell to this renderer, but the
    // standalone Studio did not. This is a derived preview, not a second
    // interior document or a mutation of authored structural changes.
    if(builder_.Model().studioViewMode!=ShipyardStudioViewMode::Exterior){
        const auto sourceKey=StudioInteriorPreviewKey::Compute(
            builder_.Model().catalog,builder_.Recipe());
        if(sourceKey!=interiorPreviewSourceKey_){
            interiorPreview_=ShipInteriorLayoutSystem{}.Plan(
                0,builder_.Model().catalog,builder_.Recipe());
            interiorPreviewSourceKey_=sourceKey;
        }
        frame.editorInteriorShell=&interiorPreview_.shell;
    }
    frame.viewportWidth=window_.GetWidth();frame.viewportHeight=window_.GetHeight();
    frame.pointerX=window_.GetPointerX();frame.pointerY=window_.GetPointerY();
    frame.elapsedSeconds=elapsed;
    renderer_.Render(frame);
    // Overlay uses the exact same projected ship position, viewport dock bounds,
    // and input picking snapshot. It never owns the blueprint or renderer.
    auto gizmo=VisibleGizmoSnapshot(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());
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
    // The material renderer can leave a GLSL program bound.  Traditional
    // glPushAttrib does NOT save GL_CURRENT_PROGRAM, so the fixed-function
    // gizmo overlay must temporarily unbind the material program and restore it.
    // This scope never changes picking or the authoritative transform model.
    const StudioOverlayProgramScope overlayProgramScope;
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
    const bool modelOnly=builder_.Recipe().modules.empty()&&!builder_.Model().modeling.recipe.primitives.empty();
    if(builder_.Recipe().modules.empty()&&!modelOnly){ShowDocumentError("Document has no ship modules or model geometry to save");return;}
    auto directory=StudioProjectPaths::Blueprints();
    if(directory.empty()){
        ShowDocumentError("Project root unavailable: cannot choose a safe blueprint directory");return;
    }
    std::error_code ec;
    std::filesystem::create_directories(directory,ec);
    if(ec){ShowDocumentError("Cannot prepare blueprint directory: "+ec.message());return;}
    std::filesystem::path selected;
    std::string error;
    std::filesystem::path suggested=documents_.Path();
    if(suggested.empty()&&modelOnly)suggested=directory/"untitled.subspace_studio";
    if(!StudioFileDialog::ChooseSaveAs(directory,suggested,selected,error)){
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
            state.definitionOverridesDirty,builder_.HasUnsavedModeling(),
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
    if(command==ShipyardBuilderCommand::WorkspaceInterior &&
       builder_.Model().workspaceMode==ShipyardWorkspaceMode::Interior){
        interiorPreviewSourceKey_.clear();
        // The builder's Interior command changes authoring mode but not view.
        // Use its EXISTING view command to enter X-Ray (visible ghost hull)
        // rather than leaving the editor showing only the opaque exterior.
        // Preserve any explicit Cutaway / Interior-only selection.
        if(builder_.Model().studioViewMode==ShipyardStudioViewMode::Exterior){
            for(int i=0;i<3 && builder_.Model().studioViewMode!=ShipyardStudioViewMode::XRay;++i)
                if(!builder_.Activate(ShipyardBuilderCommand::DccCycleStudioView))break;
        }
    }
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
    if(window_.ConsumeInputCaptureLost()){
        if(gizmoAxis_!=StudioAxis::None){builder_.CancelTransform();RestoreGizmoConstraint();}
        else if(pointerTransform_){
            if(builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets)builder_.CancelSocketTransform();
            else builder_.CancelTransform();
        }
        if(catalogDragging_)builder_.CancelCatalogDrag();
        pendingCatalogPress_=false;catalogDragging_=false;pendingCatalogIndex_=-1;
        pointerTransform_=false;gizmoAxis_=StudioAxis::None;gizmoDragged_=false;
        gizmoPixelAccum_=0;gizmoAngleDelta_=0;dockPointer_.Cancel();suppressClick_=true;
    }
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
        float radius=6.0f;Vector3 center{};
        if(builder_.Model().workspaceMode==ShipyardWorkspaceMode::Model&&!builder_.Model().modeling.recipe.primitives.empty()){
            for(const auto& p:builder_.Model().modeling.recipe.primitives){center=center+p.position;radius=std::max(radius,p.position.length()+p.size.length());}
            center=center*(1.0f/static_cast<float>(builder_.Model().modeling.recipe.primitives.size()));
        }else for(const auto& part:builder_.Recipe().modules)
            radius=std::max(radius,std::sqrt(part.x*part.x+part.y*part.y+part.z*part.z)*.24f+3.0f);
        ConstructionEditorCameraSystem::FramePreservingOrientation(constructionCamera_,center,radius);SyncConstructionCamera();
    }
    if(input_.WasPressed(InputAction::EditorFrameSelected)&&builder_.Model().workspaceMode==ShipyardWorkspaceMode::Model&&
       !builder_.Model().modeling.recipe.primitives.empty()){
        const auto i=std::min(builder_.Model().modeling.selectedPrimitiveIndex,builder_.Model().modeling.recipe.primitives.size()-1);
        const auto& p=builder_.Model().modeling.recipe.primitives[i];
        ConstructionEditorCameraSystem::FramePreservingOrientation(constructionCamera_,p.position,std::max(1.0f,p.size.length()));SyncConstructionCamera();
    }
    // NativeWindow publishes the DCC actions; standalone Studio must route
    // them itself (the game app's shortcut dispatcher is never instantiated).
    // Typing in Asset Search must never move or delete a ship component.
    const bool typing=builder_.Model().assetSearchFocused;
    const bool ctrl=window_.IsControlDown();
    if(!typing){
        if(input_.WasPressed(InputAction::DccMaximizeArea))
            RouteControl(ShipyardBuilderCommand::DccToggleMaximizeViewport);
        if(input_.WasPressed(InputAction::DccCommandSearch))
            RouteControl(ShipyardBuilderCommand::DccToggleCommandPalette);
        if(input_.WasPressed(InputAction::DccWorkspaceNext))
            RouteControl(ShipyardBuilderCommand::DccWorkspaceNext);
        if(input_.WasPressed(InputAction::DccWorkspacePrevious))
            RouteControl(ShipyardBuilderCommand::DccWorkspacePrevious);
        if(!ctrl){
            if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);
            if(input_.WasPressed(InputAction::EditorToolMove))RouteControl(ShipyardBuilderCommand::ToolMove);
            if(input_.WasPressed(InputAction::EditorToolRotate))RouteControl(ShipyardBuilderCommand::ToolRotate);
            if(scalePressed)RouteControl(ShipyardBuilderCommand::ToolScale);
            if(input_.WasPressed(InputAction::DccToggleToolbar))RouteControl(ShipyardBuilderCommand::DccToggleToolRail);
            if(input_.WasPressed(InputAction::DccToggleSidebar))RouteControl(ShipyardBuilderCommand::DccToggleSidebar);
            if(input_.WasPressed(InputAction::DccCycleAssetFilter))RouteControl(ShipyardBuilderCommand::DccNextAssetPreset);
            if(input_.WasPressed(InputAction::DccConstraintX))RouteControl(ShipyardBuilderCommand::TransformConstraintX);
            if(input_.WasPressed(InputAction::DccConstraintY))RouteControl(ShipyardBuilderCommand::TransformConstraintY);
            if(input_.WasPressed(InputAction::DccConstraintZ))RouteControl(ShipyardBuilderCommand::TransformConstraintZ);
            if(input_.WasPressed(InputAction::EditorFrameSelected))RouteControl(ShipyardBuilderCommand::FrameSelected);
            if(input_.WasPressed(InputAction::EditorDeleteModule)){
                switch(builder_.Model().workspaceMode){
                case ShipyardWorkspaceMode::Build:RouteControl(ShipyardBuilderCommand::RemoveModule);break;
                case ShipyardWorkspaceMode::Model:RouteControl(ShipyardBuilderCommand::ModelRemovePrimitive);break;
                case ShipyardWorkspaceMode::Interior:RouteControl(ShipyardBuilderCommand::InteriorRemoveElement);break;
                default:break;
                }
            }
            if(builder_.Model().workspaceMode==ShipyardWorkspaceMode::Build){
                if(input_.WasPressed(InputAction::EditorNudgeLeft))RouteControl(ShipyardBuilderCommand::NudgePort);
                if(input_.WasPressed(InputAction::EditorNudgeRight))RouteControl(ShipyardBuilderCommand::NudgeStarboard);
                if(input_.WasPressed(InputAction::EditorNudgeForward))RouteControl(ShipyardBuilderCommand::NudgeForward);
                if(input_.WasPressed(InputAction::EditorNudgeAft))RouteControl(ShipyardBuilderCommand::NudgeAft);
                if(input_.WasPressed(InputAction::EditorNudgeUp))RouteControl(ShipyardBuilderCommand::NudgeDorsal);
                if(input_.WasPressed(InputAction::EditorNudgeDown))RouteControl(ShipyardBuilderCommand::NudgeVentral);
            }
        }
    }
    float pressX=0,pressY=0;
    if(window_.ConsumePrimaryPress(pressX,pressY)){
        // A previous real drag has no click edge; do not suppress the
        // NEXT unrelated click after the user has released the mouse.
        suppressClick_=false;
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
                const auto snapshot=VisibleGizmoSnapshot(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());
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
                        // Assembly placement stores pitch/yaw/roll in packed fields;
                        // object-model primitives store true physical XYZ angles.
                        // Only assembly rotation needs the legacy Y/Z field swap.
                        const auto constrained=StudioGizmoMath::ConstraintFieldAxis(axis,
                            builder_.Model().workspaceMode==ShipyardWorkspaceMode::Model,
                            builder_.Model().transformTool==ShipyardTransformTool::Rotate);
                        builder_.SetTransformConstraint(constrained==StudioAxis::X?ShipyardTransformConstraint::X:
                            constrained==StudioAxis::Y?ShipyardTransformConstraint::Y:ShipyardTransformConstraint::Z,false);
                        pointerTransform_=true;suppressClick_=true;
                    }
                }else{
                const auto tool=builder_.Model().transformTool;
                if(builder_.Model().workspaceMode==ShipyardWorkspaceMode::Model){
                    const int picked=PickModelPrimitive(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight(),pressX,pressY);
                    if(picked>=0){
                        if(StudioToolInteractionPolicy::AllowsViewportReselection(tool))
                            RouteControl(ShipyardBuilderCommand::ModelSelectPrimitive,picked);
                        else if(StudioToolInteractionPolicy::AllowsGizmoGesture(tool)&&
                                static_cast<std::size_t>(picked)==builder_.Model().modeling.selectedPrimitiveIndex){
                            pointerTransform_=builder_.BeginSelectedTransform();if(pointerTransform_)suppressClick_=true;
                        }
                    }
                }else{
                    const int picked=NativeBattlefieldRenderer::PickShipyardModule(builder_.Model().catalog,
                        builder_.Recipe(),camera_,window_.GetWidth(),window_.GetHeight(),
                        pressX,pressY,0,0,0,.24f,.22f,true);
                    if(picked>=0){
                        if(StudioToolInteractionPolicy::AllowsViewportReselection(tool))RouteControl(ShipyardBuilderCommand::SelectPlaced,picked);
                        else if(StudioToolInteractionPolicy::AllowsGizmoGesture(tool)&&
                                static_cast<std::size_t>(picked)==builder_.Model().selectedPlacedModule){
                            pointerTransform_=builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets?
                                builder_.BeginSelectedSocketTransform():builder_.BeginSelectedTransform();
                            if(pointerTransform_)suppressClick_=true;
                        }
                    }
                }
                } // no handle: ordinary object selection remains available
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
                    // Model primitives and assembled modules share the gesture lifecycle,
                    // but model transforms use the non-destructive model recipe transaction.
                    if(builder_.Model().workspaceMode==ShipyardWorkspaceMode::Model){
                        const auto axis=gizmoAxis_;const auto tool=builder_.Model().transformTool;
                        const bool rotate=tool==ShipyardTransformTool::Rotate;
                        gizmoPixelAccum_+=StudioGizmoMath::DragScalar(gizmoStartHandle_,dragX,dragY,rotate);
                        if(std::fabs(gizmoPixelAccum_)>0.001f)gizmoDragged_=true;
                        const bool fine=window_.IsShiftDown();
                        if(builder_.ResetSelectedTransformPreview()){
                            if(tool==ShipyardTransformTool::Move){
                                const float amount=gizmoPixelAccum_*.012f/std::max(.35f,camera_.GetZoom())*(fine?.1f:1.0f);
                                builder_.TranslateSelected({axis==StudioAxis::X?amount:0,axis==StudioAxis::Y?amount:0,axis==StudioAxis::Z?amount:0},false);
                            }else if(tool==ShipyardTransformTool::Rotate){
                                const float amount=gizmoPixelAccum_*(fine?.035f:.35f);
                                builder_.RotateSelected({axis==StudioAxis::X?amount:0,axis==StudioAxis::Y?amount:0,axis==StudioAxis::Z?amount:0},false);
                                gizmoAngleDelta_=amount;
                            }else if(tool==ShipyardTransformTool::Scale){
                                const float amount=gizmoPixelAccum_*.003f*(fine?.1f:1.0f);
                                builder_.ScaleSelected({axis==StudioAxis::X?amount:0,axis==StudioAxis::Y?amount:0,axis==StudioAxis::Z?amount:0},false);
                            }
                        }
                    }else{
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
                    } // assembly gizmo transaction
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
                bool mutated=gizmoDragged_;
                if(builder_.Model().workspaceMode!=ShipyardWorkspaceMode::Model){
                    const auto& tx=builder_.Model().transform;const auto& a=tx.before;const auto& b=tx.working;
                    const auto changed=[](float x,float y){return std::fabs(x-y)>1e-5f;};
                    mutated=changed(a.x,b.x)||changed(a.y,b.y)||changed(a.z,b.z)||
                        changed(a.pitchDegrees,b.pitchDegrees)||changed(a.yawDegrees,b.yawDegrees)||
                        changed(a.rollDegrees,b.rollDegrees)||changed(a.scaleX,b.scaleX)||
                        changed(a.scaleY,b.scaleY)||changed(a.scaleZ,b.scaleZ);
                }
                if(gizmoDragged_&&mutated)builder_.CommitTransform();
                else builder_.CancelTransform();
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
