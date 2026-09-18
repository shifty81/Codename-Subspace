#include "studio/StudioApplication.h"
#include "studio/StudioSessionPolicy.h"
#include "studio/StudioProjectPaths.h"
#include "studio/StudioDocumentShortcuts.h"
#include "studio/StudioRecoveryPathPolicy.h"
#include "studio/StudioUnsavedWorkPolicy.h"
#include "studio/StudioFileDialog.h"
#include "ship_editor/ShipyardDocumentStartupSystem.h"
#include "ship_editor/ShipyardBuildSafetySystem.h"
#include "editor/EditorTransformSpaceSystem.h"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace subspace {
StudioApplication::StudioApplication():window_(input_) {}

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
    window_.SetEditorNavigationMode(true);
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
    if(StudioRecoveryPathPolicy::NeedsRecovery(unsaved.blueprint)){
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
    renderer_.Shutdown();window_.Shutdown();return exitCode;
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
    dockPointer_.Cancel();suppressClick_=true;
    std::cout<<"Studio opened "<<documents_.Path().string()<<'\n';
}
void StudioApplication::NewDocument(){
    std::string error;
    if(!documents_.New(builder_,error)){ShowDocumentError("New blocked: "+error);return;}
    pendingCatalogPress_=false;catalogDragging_=false;pointerTransform_=false;
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

void StudioApplication::HandleEscape(){
    pendingCatalogPress_=false;catalogDragging_=false;pointerTransform_=false;
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
        camera_.ClearPanOffset();camera_.SetZoom(1.0f);
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
            if(builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets)builder_.CommitSocketTransform();
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
            camera_.ZoomBy(wheel*.12f);
    }
    float dx=0,dy=0;
    if(window_.ConsumeCameraOrbitDelta(dx,dy))camera_.OrbitVisual(dx*.38f,-dy*.004f);
    if(window_.ConsumeCameraPanDelta(dx,dy))camera_.PanViewRelative(dx*.012f,dy*.012f);
    ShipyardBuildSafetySystem::SuppressFlightAndWeapons(input_);
}
} // namespace subspace
