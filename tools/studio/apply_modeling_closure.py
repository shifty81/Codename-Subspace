#!/usr/bin/env python3
"""Cumulative S15D Studio modeling closure for Codename Subspace.

Targets the exact published GREEN baseline acabf9c5. It includes the un-applied
Bulk A interaction repairs plus Model object/primitive scene, gizmo, selection,
Delete/Undo, save/reopen/recovery and truthful topology-mode normalization.
No git operations, builds or pushes are performed.
"""
from __future__ import annotations
import argparse, hashlib, json, os, sys, tempfile
from datetime import datetime, timezone
from pathlib import Path

BASELINE='acabf9c5df9e1c382eb87bc1b7264354355828f9'
PREIMAGES={
 'engine/src/studio/StudioApplication.cpp':'f62da8480ff1e1a93779cf16640b4d5c56711d41',
 'engine/src/platform/NativeWindow.cpp':'8f609e54299d71060015a7160e0e890d91d489f4',
 'engine/include/platform/NativeWindow.h':'617340066c33fc64e8dc9ad91b2e7917a7816939',
 'engine/src/ship_editor/ShipyardBuilderSystem.cpp':'b6baa09d704d1ea15e34e75d5c468c91971ecc6e',
 'engine/include/ship_editor/ShipyardBuilderSystem.h':'668026bfaf5d637c95aedc36d8a3d00546133a4f',
 'engine/src/studio/StudioAxisGizmo.cpp':'80a853c946e7aa2b541c626b6d1e654397cbfb6b',
 'engine/include/studio/StudioGizmoMath.h':'0d1d3dcaca3b456b027603cd3f9b7b1aa8a12233',
 'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp':'1de4775a94d5234ab5a4d8d698147f3a91dd0b45',
 'engine/src/studio/StudioDocumentStore.cpp':'81beac1a7c89dbcebf397f4802fc600a9d5b3cb3',
 'engine/include/studio/StudioDocumentStore.h':'1b8dd7115c0749359847abd807806511bc71eec7',
 'engine/include/studio/StudioUnsavedWorkPolicy.h':'cc3a96007a3a26450ba268768b6930aadb12d823',
 'engine/include/studio/StudioClosePolicy.h':'1874f35b3f3fedfc6b6970d71e2ed4bdc74e034a',
 'engine/src/studio/StudioFileDialog.cpp':'712ec5b185a91b55fc375695e6fbb3542be767d0',
 'engine/src/application/NativeBattlefieldRenderer.cpp':'004b1ecfd64e631cc9b71e7744909a3173e08562',
 'engine/include/modeling/ShipyardModelingSystem.h':'75e1df6d14c783d46bb11e76568424da5c05cb24',
 'engine/CMakeLists.txt':'751c2d95fe2057ddbbe00c8eac8eb2aa0441b3e6',
}
REQUIRED_NEW={
 'engine/include/studio/StudioModelDocumentCodec.h',
 'engine/src/studio/StudioModelDocumentCodec.cpp',
 'engine/tests/studio_model_document_codec_tests.cpp',
}

def blob(data:bytes)->str:
    return hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()

def once(text:str,old:str,new:str,tag:str)->str:
    n=text.count(old)
    if n!=1: raise ValueError(f'{tag}: expected exactly one source anchor, found {n}')
    return text.replace(old,new,1)

def exactly(text:str,old:str,new:str,count:int,tag:str)->str:
    n=text.count(old)
    if n!=count: raise ValueError(f'{tag}: expected {count} source anchors, found {n}')
    return text.replace(old,new)

VISIBLE_GIZMO='''namespace {
// Rendering and pointer-down share one occlusion-filtered snapshot. Hidden
// handles are never pickable through a floating panel.
StudioGizmoSnapshot VisibleGizmoSnapshot(const ShipyardBuilderRuntimeModel& model,
        const StrategicCamera& camera,int width,int height){
    auto snapshot=StudioAxisGizmo::Build(model,camera,width,height);
    if(!snapshot.visible)return snapshot;
    const auto layout=ShipyardBuilderSystem::Layout(model,width,height);
    const int dockHeight=std::max(1,static_cast<int>(layout.statusY));
    for(auto& handle:snapshot.handles){
        if(!handle.valid)continue;
        for(int sample=0;sample<=5;++sample){
            const float t=static_cast<float>(sample)/5.0f;
            const float x=handle.center.x+(handle.tip.x-handle.center.x)*t;
            const float y=handle.center.y+(handle.tip.y-handle.center.y)*t;
            if(ShipyardDockPointerSystem::CoversFloatingPanel(model.dockWorkspace,
                    width,dockHeight,layout.viewportTop,x,y)){handle.valid=false;break;}
        }
    }
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
'''

def edit_studio(src:str)->str:
    # Bulk A parity helper + model picker.
    src=once(src,'namespace subspace {\nStudioApplication::StudioApplication():window_(input_) {}',
             'namespace subspace {\n'+VISIBLE_GIZMO+'StudioApplication::StudioApplication():window_(input_) {}','shared visible gizmo/model picker')
    old='''    auto gizmo=StudioAxisGizmo::Build(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());
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
'''
    src=once(src,old,'    auto gizmo=VisibleGizmoSnapshot(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());\n','render/pick gizmo parity')
    src=once(src,'const auto snapshot=StudioAxisGizmo::Build(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());',
             'const auto snapshot=VisibleGizmoSnapshot(builder_.Model(),camera_,window_.GetWidth(),window_.GetHeight());','press gizmo parity')

    # Contextual delete and model keyboard nudges.
    old='''            if(builder_.Model().workspaceMode==ShipyardWorkspaceMode::Build){
                if(input_.WasPressed(InputAction::EditorDeleteModule))RouteControl(ShipyardBuilderCommand::RemoveModule);
                if(input_.WasPressed(InputAction::EditorNudgeLeft))RouteControl(ShipyardBuilderCommand::NudgePort);
'''
    new='''            if(input_.WasPressed(InputAction::EditorDeleteModule)){
                switch(builder_.Model().workspaceMode){
                case ShipyardWorkspaceMode::Build:RouteControl(ShipyardBuilderCommand::RemoveModule);break;
                case ShipyardWorkspaceMode::Model:RouteControl(ShipyardBuilderCommand::ModelRemovePrimitive);break;
                case ShipyardWorkspaceMode::Interior:RouteControl(ShipyardBuilderCommand::InteriorRemoveElement);break;
                default:break;
                }
            }
            if(builder_.Model().workspaceMode==ShipyardWorkspaceMode::Build){
                if(input_.WasPressed(InputAction::EditorNudgeLeft))RouteControl(ShipyardBuilderCommand::NudgePort);
'''
    src=once(src,old,new,'contextual Delete')

    # Focus/capture rollback.
    src=once(src,'void StudioApplication::HandleInput(){\n    if(input_.WasPressed(InputAction::MenuBack))HandleEscape();',
'''void StudioApplication::HandleInput(){
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
    if(input_.WasPressed(InputAction::MenuBack))HandleEscape();''','capture loss rollback')

    # Model selection/free-transform path instead of ship picker.
    old='''                }else{
                const int picked=NativeBattlefieldRenderer::PickShipyardModule(builder_.Model().catalog,
                    builder_.Recipe(),camera_,window_.GetWidth(),window_.GetHeight(),
                    pressX,pressY,0,0,0,.24f,.22f,true);
                if(picked>=0){
                    // Select changes target; other tools can free-drag only the
                    // ALREADY selected part. A miss cannot silently retarget.
                    const auto tool=builder_.Model().transformTool;
                    if(StudioToolInteractionPolicy::AllowsViewportReselection(tool)){
                        RouteControl(ShipyardBuilderCommand::SelectPlaced,picked);
                    }else if(StudioToolInteractionPolicy::AllowsGizmoGesture(tool)&&
                             static_cast<std::size_t>(picked)==builder_.Model().selectedPlacedModule){
                        pointerTransform_=builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets?
                            builder_.BeginSelectedSocketTransform():builder_.BeginSelectedTransform();
                        if(pointerTransform_)suppressClick_=true;
                    }
                }
                } // no handle: ordinary part selection remains available
'''
    new='''                }else{
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
'''
    src=once(src,old,new,'model primitive picking')

    # Axis drag: model uses total displacement transaction rather than assembly tx fields.
    anchor='''                if(gizmoAxis_!=StudioAxis::None){
                    // One pointer gesture = one authoritative ship-axis transaction.
                    // Absolute targets avoid losing sub-snap drag deltas between frames.
                    const auto& tx=builder_.Model().transform;
'''
    replacement='''                if(gizmoAxis_!=StudioAxis::None){
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
'''
    src=once(src,anchor,replacement,'model axis drag start')
    src=once(src,'''                        if(std::fabs(delta)>1e-6f)builder_.ScaleSelected(scale,fine);
                    }
                }else{
''','''                        if(std::fabs(delta)>1e-6f)builder_.ScaleSelected(scale,fine);
                    }
                    } // assembly gizmo transaction
                }else{
''','model axis drag close')

    # Release path for model transaction.
    old='''            if(gizmoAxis_!=StudioAxis::None){
                const auto& tx=builder_.Model().transform;
                const auto& a=tx.before;const auto& b=tx.working;
                const auto changed=[](float x,float y){return std::fabs(x-y)>1e-5f;};
                const bool mutated=changed(a.x,b.x)||changed(a.y,b.y)||changed(a.z,b.z)||
                    changed(a.pitchDegrees,b.pitchDegrees)||changed(a.yawDegrees,b.yawDegrees)||
                    changed(a.rollDegrees,b.rollDegrees)||changed(a.scaleX,b.scaleX)||
                    changed(a.scaleY,b.scaleY)||changed(a.scaleZ,b.scaleZ);
                if(gizmoDragged_&&mutated)builder_.CommitTransform();
                else builder_.CancelTransform(); // click/sub-snap motion is not undo history
'''
    new='''            if(gizmoAxis_!=StudioAxis::None){
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
'''
    src=once(src,old,new,'model release transaction')

    # True selected framing for model.
    old='''    if(input_.WasPressed(InputAction::EditorFrameShip)){
        // F reframes the whole assembly without resetting inspection orientation.
        // Framing individual components needs a separate precise bounds pass.
        float radius=6.0f;
        for(const auto& part:builder_.Recipe().modules){
            radius=std::max(radius,std::sqrt(part.x*part.x+part.y*part.y+part.z*part.z)*.24f+3.0f);
        }
        ConstructionEditorCameraSystem::FramePreservingOrientation(constructionCamera_,{},radius);
        SyncConstructionCamera();
    }
'''
    new='''    if(input_.WasPressed(InputAction::EditorFrameShip)){
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
'''
    src=once(src,old,new,'model frame selected/all')

    # Model save is now supported.
    src=once(src,'''void StudioApplication::SaveAsDocument(){
    if(builder_.Recipe().modules.empty()){
        ShowDocumentError("Add a ship module before saving a blueprint; model-only documents are not yet serializable");
        return;
    }
''','''void StudioApplication::SaveAsDocument(){
    const bool modelOnly=builder_.Recipe().modules.empty()&&!builder_.Model().modeling.recipe.primitives.empty();
    if(builder_.Recipe().modules.empty()&&!modelOnly){ShowDocumentError("Document has no ship modules or model geometry to save");return;}
''','allow model Save As')
    src=once(src,'if(!StudioFileDialog::ChooseSaveAs(directory,documents_.Path(),selected,error)){',
'''std::filesystem::path suggested=documents_.Path();
    if(suggested.empty()&&modelOnly)suggested=directory/"untitled.subspace_studio";
    if(!StudioFileDialog::ChooseSaveAs(directory,suggested,selected,error)){''','model save suggestion')

    # Unsaved-state policy: saved model geometry is not permanently dirty.
    src=src.replace('closing.definitionOverridesDirty,!closing.modeling.recipe.primitives.empty(),\n        closing.interiorStructure.dirty',
                    'closing.definitionOverridesDirty,builder_.HasUnsavedModeling(),\n        closing.interiorStructure.dirty')
    src=src.replace('state.definitionOverridesDirty,!state.modeling.recipe.primitives.empty(),state.interiorStructure.dirty',
                    'state.definitionOverridesDirty,builder_.HasUnsavedModeling(),state.interiorStructure.dirty')
    src=src.replace('state.definitionOverridesDirty,!state.modeling.recipe.primitives.empty(),\n            state.interiorStructure.dirty',
                    'state.definitionOverridesDirty,builder_.HasUnsavedModeling(),\n            state.interiorStructure.dirty')
    src=src.replace('return {state.dirty,state.socketOverridesDirty,state.definitionOverridesDirty,\n            !state.modeling.recipe.primitives.empty(),state.interiorStructure.dirty};',
                    'return {state.dirty&&!state.recipe.modules.empty(),state.socketOverridesDirty,state.definitionOverridesDirty,\n            builder_.HasUnsavedModeling(),state.interiorStructure.dirty};')
    src=src.replace('const StudioUnsavedWorkState unsaved{closing.dirty,closing.socketOverridesDirty,',
                    'const StudioUnsavedWorkState unsaved{closing.dirty&&!closing.recipe.modules.empty(),closing.socketOverridesDirty,')
    src=src.replace('Model/interior drafts and socket/definition edits are not stored in blueprint recovery.',
                    'Model documents now have typed recovery; interior drafts and socket/definition edits still require explicit save/review.')
    src=src.replace('Blueprint Save cannot preserve model/interior drafts or unpublished overrides. Model/interior persistence is not yet implemented: Cancel keeps Studio open; Close with recovery requires a separate explicit loss acknowledgement.',
                    'This save cannot preserve unsaved interior or unpublished socket/definition overrides. Cancel keeps Studio open; save those lanes before closing.')
    src=src.replace('WARNING: The blueprint recovery file cannot save editable model or interior drafts,',
                    'WARNING: automatic recovery cannot save unsaved interior drafts,')
    src=src.replace(' or unpublished socket/definition overrides. Those changes may be LOST.',
                    ' or unpublished socket/definition overrides. Those changes may be LOST.')
    return src


def edit_window_h(src:str)->str:
    src=once(src,'    bool IsPrimaryButtonDown() const { return _primaryButtonDown; }',
'''    bool ConsumeInputCaptureLost() { const bool lost=_inputCaptureLostPending;_inputCaptureLostPending=false;return lost; }
    bool IsPrimaryButtonDown() const { return _primaryButtonDown; }''','capture API')
    return once(src,'    bool _primaryButtonDown = false;','    bool _primaryButtonDown = false;\n    bool _inputCaptureLostPending = false;','capture storage')

def edit_window_cpp(src:str)->str:
    src=once(src,'''        case WM_KILLFOCUS:
            _inputState.Clear();
            _primaryButtonDown=false;''','''        case WM_KILLFOCUS:
            if(_primaryButtonDown)_inputCaptureLostPending=true;
            _inputState.Clear();
            _primaryButtonDown=false;''','killfocus capture signal')
    src=once(src,'''        case WM_MOUSEMOVE: {
            _pointerX''','''        case WM_CAPTURECHANGED:
            if(_primaryButtonDown){
                _inputCaptureLostPending=true;_primaryButtonDown=false;
                _primaryPressPending=false;_primaryReleasePending=false;_primaryClickPending=false;
                _primaryDragDeltaX=_primaryDragDeltaY=0.0f;
            }
            _cameraOrbitDragging=false;_cameraPanDragging=false;return 0;

        case WM_MOUSEMOVE: {
            _pointerX''','capturechanged signal')
    return once(src,'    _inputState.Clear();\n    _open = false;','    _inputState.Clear();\n    _inputCaptureLostPending=false;\n    _open = false;','capture shutdown reset')


def edit_modeling_h(src:str)->str:
    return once(src,'    std::string status = "Model workspace ready";','''    // Dirty state is revision-based so a successfully saved model remains clean
    // even though its primitives continue to exist in the document.
    std::uint32_t savedRevision = 1;
    std::string status = "Model workspace ready";''','saved model revision')


def edit_builder_h(src:str)->str:
    src=once(src,'    ModelPreviousPrimitive,\n    ModelNextPrimitive,',
             '    ModelPreviousPrimitive,\n    ModelNextPrimitive,\n    ModelSelectPrimitive,','model select command')
    src=once(src,'    bool CanRedoAuthoring() const { return !authoringRedo_.empty(); }',
'''    bool CanRedoAuthoring() const { return !authoringRedo_.empty(); }
    bool HasUnsavedModeling() const { return model_.modeling.recipe.revision!=model_.modeling.savedRevision; }
    void SetModelingState(const ShipyardModelingState& state);
    void MarkModelingSaved(const std::string& path);''','model document API')
    src=once(src,'    std::optional<ShipyardBuilderRuntimeModel> pendingTransformHistory_{};',
             '    std::optional<ShipyardBuilderRuntimeModel> pendingTransformHistory_{};\n    bool modelTransformActive_ = false;','model transaction state')
    return src


def edit_builder(src:str)->str:
    # Bulk A model undo completeness.
    src=once(src,'''    case ShipyardBuilderCommand::ModelAddFloor:
    case ShipyardBuilderCommand::ModelPreviousPurpose:''','''    case ShipyardBuilderCommand::ModelAddFloor:
    case ShipyardBuilderCommand::ModelDuplicatePrimitive:
    case ShipyardBuilderCommand::ModelRemovePrimitive:
    case ShipyardBuilderCommand::ModelAddMirrorModifier:
    case ShipyardBuilderCommand::ModelAddLinearArrayModifier:
    case ShipyardBuilderCommand::ModelAddBevelModifier:
    case ShipyardBuilderCommand::ModelPreviousPurpose:''','model history completeness')

    # Direct model primitive selection and truthful topology mode.
    src=once(src,'''        case ShipyardBuilderCommand::ModelPreviousPrimitive:
        case ShipyardBuilderCommand::ModelNextPrimitive:{if(!model_.capabilities.model)return false;constexpr int count=static_cast<int>(ModelingPrimitiveType::Pipe)+1;''',
'''        case ShipyardBuilderCommand::ModelSelectPrimitive:{
            if(!model_.capabilities.model||value<0||static_cast<std::size_t>(value)>=model_.modeling.recipe.primitives.size())return false;
            model_.modeling.selectedPrimitiveIndex=static_cast<std::size_t>(value);
            model_.modeling.selectedPrimitive=model_.modeling.recipe.primitives[static_cast<std::size_t>(value)].type;
            model_.status=std::string("Selected model shape ")+model_.modeling.recipe.primitives[static_cast<std::size_t>(value)].id;return true;}
        case ShipyardBuilderCommand::ModelPreviousPrimitive:
        case ShipyardBuilderCommand::ModelNextPrimitive:{if(!model_.capabilities.model)return false;constexpr int count=static_cast<int>(ModelingPrimitiveType::Pipe)+1;''','model selection command')
    old='''case ShipyardBuilderCommand::ModelCycleSelectionMode:{if(!model_.capabilities.model)return false;int i=(static_cast<int>(model_.modeling.selectionMode)+1)%4;model_.modeling.selectionMode=static_cast<ModelingSelectionMode>(i);model_.modeling.recipe.selectionMode=model_.modeling.selectionMode;model_.status=std::string("Model selection: ")+ShipyardModelingSystem::SelectionModeName(model_.modeling.selectionMode);return true;}'''
    if old in src:
        src=once(src,old,'''case ShipyardBuilderCommand::ModelCycleSelectionMode:{if(!model_.capabilities.model)return false;model_.modeling.selectionMode=ModelingSelectionMode::Object;model_.modeling.recipe.selectionMode=ModelingSelectionMode::Object;model_.status="OBJECT primitive editing active; vertex/edge/face topology tools are not implemented and are not exposed";return true;}''','truthful topology selection')

    # Transform transaction ownership for Model workspace.
    src=once(src,'''bool ShipyardBuilderSystem::BeginSelectedTransform(){
    if(model_.recipe.modules.empty()||model_.transformTool==ShipyardTransformTool::Select)return false;
''','''bool ShipyardBuilderSystem::BeginSelectedTransform(){
    if(model_.workspaceMode==ShipyardWorkspaceMode::Model){
        if(model_.modeling.recipe.primitives.empty()||model_.transformTool==ShipyardTransformTool::Select)return false;
        if(!pendingTransformHistory_)pendingTransformHistory_=model_;
        modelTransformActive_=true;model_.status="Model transform preview";return true;
    }
    if(model_.recipe.modules.empty()||model_.transformTool==ShipyardTransformTool::Select)return false;
''','begin model transform')
    src=once(src,'''bool ShipyardBuilderSystem::ResetSelectedTransformPreview(){
    if(!model_.transform.active||model_.transform.moduleIndex>=model_.recipe.modules.size())return false;
''','''bool ShipyardBuilderSystem::ResetSelectedTransformPreview(){
    if(modelTransformActive_&&pendingTransformHistory_){
        model_.modeling=pendingTransformHistory_->modeling;
        model_.dirty=pendingTransformHistory_->dirty;
        return true;
    }
    if(!model_.transform.active||model_.transform.moduleIndex>=model_.recipe.modules.size())return false;
''','reset model preview')
    src=once(src,'''bool ShipyardBuilderSystem::CommitTransform(){if(!model_.transform.active)return false;const auto index=model_.transform.moduleIndex;auto p=ShipyardTransformSystem::Commit(model_.transform);''',
'''bool ShipyardBuilderSystem::CommitTransform(){
    if(modelTransformActive_){
        modelTransformActive_=false;
        if(pendingTransformHistory_){
            if(model_.modeling.recipe.revision!=pendingTransformHistory_->modeling.recipe.revision)PushAuthoringSnapshot(*pendingTransformHistory_);
            pendingTransformHistory_.reset();
        }
        model_.status="Model transform committed";return true;
    }
    if(!model_.transform.active)return false;const auto index=model_.transform.moduleIndex;auto p=ShipyardTransformSystem::Commit(model_.transform);''','commit model transform')
    src=once(src,'''bool ShipyardBuilderSystem::CancelTransform(){if(!model_.transform.active)return false;const auto index=model_.transform.moduleIndex;auto p=ShipyardTransformSystem::Cancel(model_.transform);''',
'''bool ShipyardBuilderSystem::CancelTransform(){
    if(modelTransformActive_){
        modelTransformActive_=false;
        if(pendingTransformHistory_){model_.modeling=pendingTransformHistory_->modeling;model_.dirty=pendingTransformHistory_->dirty;pendingTransformHistory_.reset();}
        model_.status="Model transform cancelled";return true;
    }
    if(!model_.transform.active)return false;const auto index=model_.transform.moduleIndex;auto p=ShipyardTransformSystem::Cancel(model_.transform);''','cancel model transform')

    # Model document API near save markers.
    anchor='''void ShipyardBuilderSystem::MarkApplied(){model_.dirty=false;model_.status="Applied to player ship visual blueprint";}
void ShipyardBuilderSystem::MarkSaved(const std::string& path){'''
    replacement='''void ShipyardBuilderSystem::MarkApplied(){model_.dirty=false;model_.status="Applied to player ship visual blueprint";}
void ShipyardBuilderSystem::SetModelingState(const ShipyardModelingState& state){
    model_.modeling=state;model_.workspaceMode=ShipyardWorkspaceMode::Model;
    model_.transformTool=ShipyardTransformTool::Select;model_.transformSpace=ShipyardTransformSpace::Ship;
    model_.dirty=false;modelTransformActive_=false;pendingTransformHistory_.reset();
    model_.status="Model document loaded";
}
void ShipyardBuilderSystem::MarkModelingSaved(const std::string& path){
    model_.modeling.savedRevision=model_.modeling.recipe.revision;
    if(model_.recipe.modules.empty())model_.dirty=false;
    model_.status="Saved model document: "+path;
}
void ShipyardBuilderSystem::MarkSaved(const std::string& path){'''
    src=once(src,anchor,replacement,'model document builder API')
    return src


def edit_axis(src:str)->str:
    # Replace early guard to allow either Assembly Build or Model primitive context.
    src=once(src,'''    if(width<=0||height<=0||model.recipe.modules.empty()||!model.dcc.showGizmos||
       model.workspaceMode!=ShipyardWorkspaceMode::Build||model.testWorkspaceActive||
       model.inspectorTab==ShipyardInspectorTab::Sockets||
       model.dragPreview.active||model.dragPreview.staged)return out;''',
'''    const bool modelMode=model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.modeling.recipe.primitives.empty();
    const bool assemblyMode=model.workspaceMode==ShipyardWorkspaceMode::Build&&!model.recipe.modules.empty();
    if(width<=0||height<=0||(!modelMode&&!assemblyMode)||!model.dcc.showGizmos||model.testWorkspaceActive||
       model.inspectorTab==ShipyardInspectorTab::Sockets||model.dragPreview.active||model.dragPreview.staged)return out;''','model gizmo mode guard')
    anchor='''    const auto& part=model.recipe.modules[std::min(model.selectedPlacedModule,model.recipe.modules.size()-1)];
    const VisualModuleSource* source=nullptr;
'''
    replacement='''    if(modelMode){
        const auto& p=model.modeling.recipe.primitives[std::min(model.modeling.selectedPrimitiveIndex,model.modeling.recipe.primitives.size()-1)];
        out.readout.position={{p.position.x,p.position.y,p.position.z}};
        out.readout.rotationDegrees={{p.rotationDegrees.x,p.rotationDegrees.y,p.rotationDegrees.z}};
        out.readout.scalePercent={{100.0f,100.0f,100.0f}};
        out.readout.nominalLocalMeters={{p.size.x,p.size.y,p.size.z}};out.readout.nominalDimensionsAvailable=true;out.readoutVisible=true;
        if(model.transformTool==ShipyardTransformTool::Select)return out;
        const auto center=NativeBattlefieldRenderer::WorldToScreen(p.position,width,height,camera);
        if(!center.visible||center.x<out.viewportLeft+18||center.x>out.viewportRight-18||center.y<out.viewportTop+18||center.y>out.viewportBottom-18)return out;
        const Vector3 basis[3]={{1,0,0},{0,1,0},{0,0,1}};
        for(int i=0;i<3;++i){
            const auto projected=NativeBattlefieldRenderer::WorldToScreen(p.position+basis[i]*std::max(1.0f,p.size.length()*.55f),width,height,camera);
            auto& h=out.handles[static_cast<std::size_t>(i)];h.axis=static_cast<StudioAxis>(i);h.center={center.x,center.y};
            if(!projected.visible)continue;const StudioPoint raw{projected.x-center.x,projected.y-center.y};if(StudioGizmoMath::Length(raw)<1.8f)continue;
            const auto dir=StudioGizmoMath::Unit(raw);float length=68.0f;
            const float left=out.viewportLeft+10,right=out.viewportRight-10,top=out.viewportTop+10,bottom=out.viewportBottom-10;
            if(dir.x>.001f)length=std::min(length,(right-center.x)/dir.x);else if(dir.x<-.001f)length=std::min(length,(left-center.x)/dir.x);
            if(dir.y>.001f)length=std::min(length,(bottom-center.y)/dir.y);else if(dir.y<-.001f)length=std::min(length,(top-center.y)/dir.y);
            if(!std::isfinite(length)||length<24)continue;h.tip={center.x+dir.x*length,center.y+dir.y*length};h.valid=true;out.visible=true;
        }
        return out;
    }
    const auto& part=model.recipe.modules[std::min(model.selectedPlacedModule,model.recipe.modules.size()-1)];
    const VisualModuleSource* source=nullptr;
'''
    src=once(src,anchor,replacement,'model gizmo body')
    # Bulk A edge fixes for assembly.
    src=once(src,'''    if(!center.visible||center.x<out.viewportLeft+78||center.x>out.viewportRight-78||
       center.y<out.viewportTop+78||center.y>out.viewportBottom-78)return out;''',
'''    if(!center.visible||center.x<out.viewportLeft+18||center.x>out.viewportRight-18||
       center.y<out.viewportTop+18||center.y>out.viewportBottom-18)return out;''','assembly edge pivot')
    src=once(src,'''        const auto dir=StudioGizmoMath::Unit({projected.x-center.x,projected.y-center.y});
        if(StudioGizmoMath::Length(dir)<.8f)continue;
        h.tip={center.x+dir.x*68.0f,center.y+dir.y*68.0f};''',
'''        const StudioPoint projectedDelta{projected.x-center.x,projected.y-center.y};
        if(StudioGizmoMath::Length(projectedDelta)<1.8f)continue;
        const auto dir=StudioGizmoMath::Unit(projectedDelta);
        float length=68.0f;const float left=out.viewportLeft+10,right=out.viewportRight-10,top=out.viewportTop+10,bottom=out.viewportBottom-10;
        if(dir.x>.001f)length=std::min(length,(right-center.x)/dir.x);else if(dir.x<-.001f)length=std::min(length,(left-center.x)/dir.x);
        if(dir.y>.001f)length=std::min(length,(bottom-center.y)/dir.y);else if(dir.y<-.001f)length=std::min(length,(top-center.y)/dir.y);
        if(!std::isfinite(length)||length<24.0f)continue;
        h.tip={center.x+dir.x*length,center.y+dir.y*length};''','assembly projected axis/clip')
    return src


def edit_math(src:str)->str:
    return once(src,'''        const float t=std::clamp(Dot(q,d)/len2,.24f,1.12f);
        const StudioPoint nearestPoint{h.center.x+d.x*t,h.center.y+d.y*t};
        return Length(Delta({px,py},nearestPoint))<=radius;''',
'''        const float tipDistance=Length(Delta({px,py},h.tip));
        if(tipDistance<=radius+6.0f)return true; // marker is intentionally easier than the shaft
        const float t=Dot(q,d)/len2;if(t<.22f||t>1.12f)return false;
        const StudioPoint nearestPoint{h.center.x+d.x*t,h.center.y+d.y*t};
        return Length(Delta({px,py},nearestPoint))<=std::max(8.0f,radius-3.0f);''','marker-first gizmo hit')


def edit_visible(src:str)->str:
    # Model outliner navigation uses the active object collection, not ship-only count.
    src=once(src,'''    case ShipyardBuilderCommand::DccOutlinerPrevious:
    case ShipyardBuilderCommand::DccOutlinerNext:{
        const auto count=model_.recipe.modules.size();''',
'''    case ShipyardBuilderCommand::DccOutlinerPrevious:
    case ShipyardBuilderCommand::DccOutlinerNext:{
        const auto count=model_.workspaceMode==ShipyardWorkspaceMode::Model?model_.modeling.recipe.primitives.size():model_.recipe.modules.size();''','model outliner navigation count')
    # Model tool rail = actual selection/transforms + create/delete.
    old='''        if(model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.testWorkspaceActive){
            add(ShipyardBuilderCommand::ModelAddBox,0,tx,railButtonsY,tw,th,"BOX",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelAddWedge,0,tx,railButtonsY+(th+gap),tw,th,"WEDGE",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelAddCylinder,0,tx,railButtonsY+2*(th+gap),tw,th,"CYL",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelAddDoor,0,tx,railButtonsY+3*(th+gap),tw,th,"DOOR",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelDuplicatePrimitive,0,tx,railButtonsY+4*(th+gap),tw,th,"COPY",false,!model.modeling.recipe.primitives.empty());
            add(ShipyardBuilderCommand::ModelRemovePrimitive,0,tx,railButtonsY+5*(th+gap),tw,th,"DEL",false,!model.modeling.recipe.primitives.empty());
'''
    new='''        if(model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.testWorkspaceActive){
            const bool hasShape=!model.modeling.recipe.primitives.empty();
            add(ShipyardBuilderCommand::ToolSelect,0,tx,railButtonsY,tw,th,"SELECT",model.transformTool==ShipyardTransformTool::Select,true);
            add(ShipyardBuilderCommand::ToolMove,0,tx,railButtonsY+(th+gap),tw,th,"MOVE",model.transformTool==ShipyardTransformTool::Move,hasShape);
            add(ShipyardBuilderCommand::ToolRotate,0,tx,railButtonsY+2*(th+gap),tw,th,"ROTATE",model.transformTool==ShipyardTransformTool::Rotate,hasShape);
            add(ShipyardBuilderCommand::ToolScale,0,tx,railButtonsY+3*(th+gap),tw,th,"SCALE",model.transformTool==ShipyardTransformTool::Scale,hasShape);
            add(ShipyardBuilderCommand::ModelAddBox,0,tx,railButtonsY+4*(th+gap),tw,th,"ADD BOX",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelRemovePrimitive,0,tx,railButtonsY+5*(th+gap),tw,th,"DEL",false,hasShape);
'''
    src=once(src,old,new,'model tool rail')
    src=once(src,'ShipyardBuilderCommand::ModelCycleSelectionMode,"SELECT MODE",false,hasShape,ay);',
             'ShipyardBuilderCommand::ModelDuplicatePrimitive,"DUPLICATE",false,hasShape,ay);','model properties truthful action')
    # Model outliner rows replace ship rows while Model workspace is active.
    old='''            const auto rows=ShipyardDccUiSystem::BuildOutlinerRows(model.catalog,model.recipe,model.selectedPlacedModule,model.dcc.outlinerMode);
            const std::size_t page=l.compact?4u:5u;const std::size_t start=rows.empty()?0:std::min(model.placedScrollStart,rows.size()>page?rows.size()-page:0u);
            const float rowsY=l.outlinerY+metrics.panelHeaderHeight*s+31.0f*s;
            for(std::size_t i=0;i<page&&start+i<rows.size();++i){const auto& item=rows[start+i];std::string displayLabel=item.label;if(item.moduleIndex<model.recipe.modules.size()){const auto& placedId=model.recipe.modules[item.moduleIndex].moduleId;const auto found=std::find_if(model.catalog.begin(),model.catalog.end(),[&](const auto& record){return record.source.moduleId==placedId;});if(found!=model.catalog.end())displayLabel=FriendlyModuleLabel(*found);}std::string label=model.dcc.outlinerMode==ShipyardDccOutlinerMode::Hierarchy?std::string(item.depth*2,' '):std::string{};if(model.dcc.outlinerMode!=ShipyardDccOutlinerMode::Hierarchy)label=item.group+" | ";label+=(item.attached?"|_ ":"o  ")+displayLabel;add(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(item.moduleIndex),rx,rowsY+i*(l.rowHeight+gap),rw,l.rowHeight,label,item.selected,true);}
'''
    new='''            const std::size_t page=l.compact?4u:5u;const float rowsY=l.outlinerY+metrics.panelHeaderHeight*s+31.0f*s;
            if(model.workspaceMode==ShipyardWorkspaceMode::Model){
                const auto& shapes=model.modeling.recipe.primitives;const std::size_t start=shapes.empty()?0:std::min(model.placedScrollStart,shapes.size()>page?shapes.size()-page:0u);
                for(std::size_t i=0;i<page&&start+i<shapes.size();++i){const auto index=start+i;const auto& p=shapes[index];
                    const std::string label=std::to_string(index+1)+" | "+ShipyardModelingSystem::PrimitiveName(p.type)+" | "+p.id;
                    add(ShipyardBuilderCommand::ModelSelectPrimitive,static_cast<int>(index),rx,rowsY+i*(l.rowHeight+gap),rw,l.rowHeight,label,index==model.modeling.selectedPrimitiveIndex,true);}
            }else{
                const auto rows=ShipyardDccUiSystem::BuildOutlinerRows(model.catalog,model.recipe,model.selectedPlacedModule,model.dcc.outlinerMode);
                const std::size_t start=rows.empty()?0:std::min(model.placedScrollStart,rows.size()>page?rows.size()-page:0u);
                for(std::size_t i=0;i<page&&start+i<rows.size();++i){const auto& item=rows[start+i];std::string displayLabel=item.label;if(item.moduleIndex<model.recipe.modules.size()){const auto& placedId=model.recipe.modules[item.moduleIndex].moduleId;const auto found=std::find_if(model.catalog.begin(),model.catalog.end(),[&](const auto& record){return record.source.moduleId==placedId;});if(found!=model.catalog.end())displayLabel=FriendlyModuleLabel(*found);}std::string label=model.dcc.outlinerMode==ShipyardDccOutlinerMode::Hierarchy?std::string(item.depth*2,' '):std::string{};if(model.dcc.outlinerMode!=ShipyardDccOutlinerMode::Hierarchy)label=item.group+" | ";label+=(item.attached?"|_ ":"o  ")+displayLabel;add(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(item.moduleIndex),rx,rowsY+i*(l.rowHeight+gap),rw,l.rowHeight,label,item.selected,true);}
            }
'''
    return once(src,old,new,'model outliner')


def edit_document_h(src:str)->str:
    return once(src,'#include "ship_editor/ShipyardBuilderSystem.h"',
                '#include "ship_editor/ShipyardBuilderSystem.h"\n#include "studio/StudioModelDocumentCodec.h"','model codec include')


def edit_document(src:str)->str:
    src=once(src,'#include "studio/StudioUnsavedWorkPolicy.h"',
             '#include "studio/StudioUnsavedWorkPolicy.h"\n#include "studio/StudioModelDocumentCodec.h"','codec include')
    # Open model extension before blueprint loader.
    anchor='''    ShipBlueprintDocument document;
    if(!ShipBlueprintLibrarySystem::Load(source.string(),document,&error))return false;
'''
    replacement='''    if(StudioModelDocumentCodec::IsStudioModelPath(source)){
        ShipyardModelingState modeling;if(!StudioModelDocumentCodec::Load(source,modeling,error))return false;
        const auto catalog=builder.Model().catalog;const auto layout=builder.Model().dockWorkspace;
        builder.Initialize(catalog,ShipyardDocumentStartupSystem::EmptyDocument());builder.MutableDockWorkspace()=layout;
        builder.SetLiveApplyEnabled(false,true);builder.SetModelingState(modeling);path_=source;builder.MarkModelingSaved(path_.filename().string());return true;
    }
    ShipBlueprintDocument document;
    if(!ShipBlueprintLibrarySystem::Load(source.string(),document,&error))return false;
'''
    src=once(src,anchor,replacement,'open model doc')
    # SaveAs model branch before extension restriction.
    src=once(src,'''    if(destination.empty()){error="Choose a blueprint path";return false;}
    if(destination.extension()!=".subspace_ship") {error="Studio saves .subspace_ship blueprints";return false;}
    if(builder.Recipe().modules.empty()){error="Empty document: add a module before saving";return false;}
''','''    if(destination.empty()){error="Choose a Studio document path";return false;}
    if(StudioModelDocumentCodec::IsStudioModelPath(destination)){
        if(builder.Model().modeling.recipe.primitives.empty()){error="Model document has no geometry";return false;}
        const bool overwrite=destination==path_;
        if(!StudioModelDocumentCodec::Save(destination,builder.Model().modeling,overwrite,error))return false;
        path_=destination;builder.MarkModelingSaved(path_.filename().string());return true;
    }
    if(destination.extension()!=".subspace_ship") {error="Studio saves .subspace_ship or .subspace_studio documents";return false;}
    if(builder.Recipe().modules.empty()){error="Empty ship document: add a module before saving";return false;}
''','save model doc')
    # Save chooses extension for new model-only doc.
    src=once(src,'''bool StudioDocumentStore::Save(ShipyardBuilderSystem& builder,std::string& error){
    const auto destination=path_.empty()?NewDraftPath():path_;
''','''bool StudioDocumentStore::Save(ShipyardBuilderSystem& builder,std::string& error){
    std::filesystem::path destination=path_;
    if(destination.empty()&&!builder.Model().modeling.recipe.primitives.empty()&&builder.Recipe().modules.empty()){
        const auto base=StudioProjectPaths::Blueprints();if(base.empty()){error="Unable to locate Studio document directory";return false;}
        const auto tick=std::chrono::system_clock::now().time_since_epoch().count();destination=base/("studio_model_"+std::to_string(tick)+".subspace_studio");
    }else if(destination.empty())destination=NewDraftPath();
''','save model default')
    # Model-only exit recovery.
    src=once(src,'''    if(builder.Recipe().modules.empty()){
        error="No placed modules: blueprint recovery cannot store an editable model-only draft";
        return false;
    }
''','''    if(builder.Recipe().modules.empty()){
        if(builder.Model().modeling.recipe.primitives.empty()){error="No ship or model geometry to recover";return false;}
        const auto base=StudioProjectPaths::Blueprints();if(base.empty()){error="Project root missing: recovery cannot choose a safe destination";return false;}
        const auto tick=std::chrono::system_clock::now().time_since_epoch().count();
        for(unsigned i=0;i<1000;++i){const auto candidate=base/("studio_model_recovery_"+std::to_string(tick)+"_"+std::to_string(i)+".subspace_studio");std::error_code ec;if(std::filesystem::exists(candidate,ec)||ec)continue;
            if(StudioModelDocumentCodec::Save(candidate,builder.Model().modeling,false,error)){recovered=candidate;return true;}return false;}
        error="Could not reserve a unique model recovery filename";return false;
    }
''','model exit recovery')
    # Open/New both use revision dirty authority rather than primitive existence.
    src=exactly(src,'state.definitionOverridesDirty,!state.modeling.recipe.primitives.empty(),\n            state.interiorStructure.dirty',
                    'state.definitionOverridesDirty,builder.HasUnsavedModeling(),\n            state.interiorStructure.dirty',2,'document model dirty guards')
    return src


def edit_unsaved_policy(src:str)->str:
    return once(src,'return s.socketOverrides||s.definitionOverrides||s.modelDraft||s.interiorDraft;',
                'return s.socketOverrides||s.definitionOverrides||s.interiorDraft;','model recovery is now supported')

def edit_close_policy(src:str)->str:
    return once(src,'return state.socketDirty||state.definitionDirty||state.modelDraft||state.interiorDraft;',
                'return state.socketDirty||state.definitionDirty||state.interiorDraft;','model close recovery support')

def edit_dialog(src:str)->str:
    src=once(src,'''constexpr wchar_t kBlueprintFilter[] =
    L"Subspace blueprints (*.subspace_ship)\\0*.subspace_ship\\0All files (*.*)\\0*.*\\0";''',
'''constexpr wchar_t kBlueprintFilter[] =
    L"Subspace Studio documents (*.subspace_studio;*.subspace_ship)\\0*.subspace_studio;*.subspace_ship\\0"
    L"Model documents (*.subspace_studio)\\0*.subspace_studio\\0Ship blueprints (*.subspace_ship)\\0*.subspace_ship\\0All files (*.*)\\0*.*\\0";''','dialog filter')
    src=once(src,'request.lpstrDefExt=L"subspace_ship";',
'''const bool modelDefault=initial.extension()==".subspace_studio";
    request.lpstrDefExt=modelDefault?L"subspace_studio":L"subspace_ship";''','dialog default extension')
    return src


def edit_renderer(src:str)->str:
    src=once(src,'#include "editor/EditorGizmoSystem.h"',
             '#include "editor/EditorGizmoSystem.h"\n#include "modeling/ShipyardModelingSystem.h"','model renderer include')
    # Insert helper immediately after DrawBox.
    marker='''Rgba PlanetColor(PlanetType t) {'''
    helper=r'''void DrawStudioModelScene(const NativeBattlefieldFrame& frame){
    if(!frame.shipBuilder||frame.shipBuilder->workspaceMode!=ShipyardWorkspaceMode::Model||
       frame.shipBuilder->modeling.recipe.primitives.empty())return;
    auto preview=frame.shipBuilder->modeling.recipe;
    // Unsupported modifiers are stored honestly but must not make the editable
    // source geometry disappear from the viewport.
    for(auto& m:preview.modifiers)if(m.type!=ModelingModifierType::Mirror&&m.type!=ModelingModifierType::LinearArray)m.enabled=false;
    const auto asset=ShipyardModelingSystem::BakeCanonicalAsset(preview,"studio.model.preview");if(asset.meshes.empty())return;
    const auto layout=ShipyardBuilderSystem::Layout(*frame.shipBuilder,frame.viewportWidth,frame.viewportHeight);
    glEnable(GL_SCISSOR_TEST);glScissor(static_cast<GLint>(layout.viewportLeft),static_cast<GLint>(frame.viewportHeight-layout.viewportBottom),
        static_cast<GLsizei>(layout.viewportRight-layout.viewportLeft),static_cast<GLsizei>(layout.viewportBottom-layout.viewportTop));
    DisableShader();glEnable(GL_DEPTH_TEST);glDisable(GL_TEXTURE_2D);glEnable(GL_LIGHTING);
    for(std::size_t ni=0;ni<asset.nodes.size();++ni){const auto& node=asset.nodes[ni];if(node.meshIndex==assets::kInvalidAssetIndex||node.meshIndex>=asset.meshes.size())continue;
        glPushMatrix();glMultMatrixf(node.localTransform.value.data());const bool selected=ni==frame.shipBuilder->modeling.selectedPrimitiveIndex;
        SetMaterial(selected?Rgba{.22f,.66f,.82f,1.0f}:Rgba{.34f,.40f,.46f,1.0f},selected?54.0f:28.0f,0.0f,SpaceMaterialKind::ShipHull);
        for(const auto& part:asset.meshes[node.meshIndex].primitives){glBegin(GL_TRIANGLES);for(const auto idx:part.indices)if(idx<part.vertices.size()){const auto& v=part.vertices[idx];glNormal3f(v.normal.x,v.normal.y,v.normal.z);glVertex3f(v.position.x,v.position.y,v.position.z);}glEnd();}
        if(selected){glDisable(GL_LIGHTING);glColor4f(.32f,.86f,1.0f,.92f);glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);glLineWidth(1.6f);
            for(const auto& part:asset.meshes[node.meshIndex].primitives){glBegin(GL_TRIANGLES);for(const auto idx:part.indices)if(idx<part.vertices.size()){const auto& v=part.vertices[idx];glVertex3f(v.position.x,v.position.y,v.position.z);}glEnd();}
            glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glEnable(GL_LIGHTING);}
        glPopMatrix();}
    glDisable(GL_SCISSOR_TEST);
}

'''
    src=once(src,marker,helper+marker,'model scene renderer helper')
    src=once(src,'    if(frame.standaloneShipyard&&frame.shipBuilderRecipe){\n',
'''    if(frame.standaloneShipyard&&frame.shipBuilderRecipe){
        DrawStudioModelScene(frame);
''','invoke model scene')
    return src


def edit_cmake(src:str)->str:
    src=once(src,'    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioNativeCloseGuard.cpp"\n    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioAxisGizmo.cpp"',
             '    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioNativeCloseGuard.cpp"\n    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioModelDocumentCodec.cpp"\n    "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioAxisGizmo.cpp"','studio codec source')
    anchor='''    add_executable(subspace_studio_document_roundtrip_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_document_roundtrip_tests.cpp")
    target_link_libraries(subspace_studio_document_roundtrip_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceStudioDocumentRoundtripTests COMMAND subspace_studio_document_roundtrip_tests)
'''
    addition=anchor+'''    add_executable(subspace_studio_model_document_codec_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/studio_model_document_codec_tests.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/studio/StudioModelDocumentCodec.cpp")
    target_link_libraries(subspace_studio_model_document_codec_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceStudioModelDocumentCodecTests COMMAND subspace_studio_model_document_codec_tests)
    add_test(NAME SubspaceStudioModelingClosureSourceGate COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/static-gates/studio_modeling_closure_authority.cmake)
'''
    return once(src,anchor,addition,'codec CTest')

EDITORS={
 'engine/src/studio/StudioApplication.cpp':edit_studio,
 'engine/src/platform/NativeWindow.cpp':edit_window_cpp,
 'engine/include/platform/NativeWindow.h':edit_window_h,
 'engine/include/modeling/ShipyardModelingSystem.h':edit_modeling_h,
 'engine/include/ship_editor/ShipyardBuilderSystem.h':edit_builder_h,
 'engine/src/ship_editor/ShipyardBuilderSystem.cpp':edit_builder,
 'engine/src/studio/StudioAxisGizmo.cpp':edit_axis,
 'engine/include/studio/StudioGizmoMath.h':edit_math,
 'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp':edit_visible,
 'engine/include/studio/StudioDocumentStore.h':edit_document_h,
 'engine/include/studio/StudioUnsavedWorkPolicy.h':edit_unsaved_policy,
 'engine/include/studio/StudioClosePolicy.h':edit_close_policy,
 'engine/src/studio/StudioDocumentStore.cpp':edit_document,
 'engine/src/studio/StudioFileDialog.cpp':edit_dialog,
 'engine/src/application/NativeBattlefieldRenderer.cpp':edit_renderer,
 'engine/CMakeLists.txt':edit_cmake,
}

def stage(root:Path,strict:bool=True):
    if strict and not (root/'.git').exists():raise ValueError('Git checkout required; extracted source staging is not a valid mutation target')
    for rel in REQUIRED_NEW:
        if not (root/rel).is_file():raise ValueError(f'PCC payload prerequisite missing: {rel}')
    pending={};receipts={}
    for rel,editor in EDITORS.items():
        path=root/rel
        if not path.is_file():raise ValueError(f'missing source: {rel}')
        original=path.read_bytes()
        if b'\r\n' in original:raise ValueError(f'line ending conflict: {rel}')
        h=blob(original)
        if strict and h!=PREIMAGES[rel]:raise ValueError(f'PREIMAGE_CONFLICT {rel}: expected {PREIMAGES[rel]}, got {h}; zero writes')
        updated=editor(original.decode('utf-8')).encode('utf-8')
        if updated==original:raise ValueError(f'no source change produced: {rel}')
        pending[rel]=(original,updated)
        receipts[rel]={'oldGitBlob':h,'oldSha256':hashlib.sha256(original).hexdigest(),'newSha256':hashlib.sha256(updated).hexdigest(),'bytes':len(updated)}
    return pending,receipts

def apply(root:Path,pending:dict,receipts:dict):
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    backup=root/'.subspace'/'recovery'/'studio-modeling-closure'/stamp;backup.mkdir(parents=True,exist_ok=False)
    for rel,(original,_) in pending.items():
        p=backup/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(original)
    receipt=backup/'RECEIPT.json';meta={'schema':'subspace.studio-modeling-closure.v1','status':'BACKED_UP','baselineGitCommit':BASELINE,'files':receipts,'backupRoot':str(backup)}
    def save_receipt():receipt.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
    save_receipt();written=[]
    try:
        for rel,(original,_) in pending.items():
            if (root/rel).read_bytes()!=original:raise RuntimeError('CONCURRENT_EDIT '+rel)
        for rel,(original,updated) in pending.items():
            path=root/rel;fd,tmp=tempfile.mkstemp(prefix='.studio-modeling-',dir=path.parent)
            try:
                with os.fdopen(fd,'wb') as f:f.write(updated);f.flush();os.fsync(f.fileno())
                os.replace(tmp,path)
            finally:
                if os.path.exists(tmp):os.unlink(tmp)
            written.append(rel)
            if hashlib.sha256(path.read_bytes()).hexdigest()!=receipts[rel]['newSha256']:raise RuntimeError('POSTWRITE_HASH_MISMATCH '+rel)
    except Exception:
        for rel in written:(root/rel).write_bytes((backup/rel).read_bytes())
        meta['status']='ROLLED_BACK';save_receipt();raise
    meta['status']='APPLIED';save_receipt();return receipt

def main(argv=None):
    p=argparse.ArgumentParser(description='Subspace Studio cumulative object-modeling closure')
    g=p.add_mutually_exclusive_group(required=True);g.add_argument('--check',action='store_true');g.add_argument('--apply',action='store_true')
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2]);a=p.parse_args(argv)
    try:
        pending,receipts=stage(a.root.resolve())
        if a.check:
            print('STUDIO_MODELING_CLOSURE_PREIMAGE_CHECK PASS (read-only)')
            for rel,row in receipts.items():print(rel,row['oldGitBlob'],row['newSha256'])
        else:
            receipt=apply(a.root.resolve(),pending,receipts)
            print('STUDIO_MODELING_CLOSURE_APPLIED backup and receipt:',receipt)
            print('NEXT: PCC Full Quality Gate, then hands-on Model BOX/select/move/rotate/scale/Delete/Undo/save/reopen tests.')
            print('NOTE: vertex/edge/face topology editing is intentionally not claimed by this object-modeling closure.')
    except (OSError,UnicodeError,ValueError,RuntimeError) as e:
        print('STUDIO_MODELING_CLOSURE_BLOCKED:',e,file=sys.stderr);return 1
    return 0
if __name__=='__main__':raise SystemExit(main())
