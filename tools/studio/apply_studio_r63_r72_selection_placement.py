#!/usr/bin/env python3
"""Guarded R63-R72 Studio selection/placement ergonomics normalization.

Requires R53-R62. Edits are exact-preimage bounded, idempotent, transactional,
and backed up before replacement. The migration deliberately keeps legacy
branch-delete behavior behind its old command while routing normal Studio Delete
through a new safe selection command.
"""
from __future__ import annotations
import argparse, hashlib, json, os, shutil, sys
from datetime import datetime, timezone
from pathlib import Path
from studio_verified_normalized_state import verify_complete_r82r1


def sha256(data: bytes)->str: return hashlib.sha256(data).hexdigest()
def replace_exact(text:str,old:str,new:str,label:str):
    if new in text:return text,False
    n=text.count(old)
    if n!=1:raise RuntimeError(f"{label}: expected one approved preimage, found {n}")
    return text.replace(old,new,1),True

def require(text:str,token:str,label:str):
    if token not in text:raise RuntimeError(f"{label}: required token missing: {token}")


def main()->int:
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);ap.add_argument('--apply',action='store_true');args=ap.parse_args()
    root=Path(args.root).resolve()
    # An R53+ migration extends R33 postimages; do not replay obsolete exact
    # preimages if the full R82R1 state is independently source-gate proven.
    if verify_complete_r82r1(root):
        print("r63_r72_selection_placement: complete R82R1 normalized source verified; historical migration no-op.")
        return 0
    files={
      'input_h':root/'engine/include/input/InputState.h',
      'window_cpp':root/'engine/src/platform/NativeWindow.cpp',
      'builder_h':root/'engine/include/ship_editor/ShipyardBuilderSystem.h',
      'builder_cpp':root/'engine/src/ship_editor/ShipyardBuilderSystem.cpp',
      'drag_h':root/'engine/include/ship_editor/ShipyardDragDropSystem.h',
      'app_cpp':root/'engine/src/studio/StudioApplication.cpp',
      'visible_cpp':root/'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp',
      'policy_h':root/'engine/include/studio/StudioPlacementWorkflowPolicy.h',
    }
    for k,p in files.items():
        if not p.is_file():raise RuntimeError(f"R63-R72 missing source {k}: {p}")
    original={k:p.read_text(encoding='utf-8') for k,p in files.items()}
    require(original['input_h'],'DccCycleTransformSpace','R53-R62 prerequisite input')
    require(original['app_cpp'],'InputAction::DccCycleTransformSpace','R53-R62 prerequisite Studio routing')
    require(original['builder_cpp'],'Transform space changed; transient axis/local constraint cleared','R53-R62 prerequisite builder state')
    require(original['policy_h'],'NormalDeletePreservesDescendants','R63-R72 package policy')
    changed=dict(original);touched=[]
    def edit(k,old,new,label):
        nonlocal changed,touched
        changed[k],did=replace_exact(changed[k],old,new,label)
        if did and k not in touched:touched.append(k)

    # R65: append action to preserve every historical InputAction value.
    edit('input_h',
         '    DccCycleTransformSpace,\n    Count',
         '    DccCycleTransformSpace,\n    DccDuplicateSelection,\n    Count',
         'append DCC duplicate action')
    edit('window_cpp',
         "        case 'D': _inputState.SetAction(InputAction::StrafeRight, down); break;",
         "        case 'D':\n            _inputState.SetAction(InputAction::StrafeRight, down);\n            if(down&&_shiftDown)_inputState.SetAction(InputAction::DccDuplicateSelection,true);\n            else if(!down)_inputState.SetAction(InputAction::DccDuplicateSelection,false);\n            break;",
         'Shift+D duplicate binding')

    # R70: append public editor commands after the contiguous File/Edit/View/Help
    # menu range so existing command identifiers and menu arithmetic stay stable.
    edit('builder_h',
         '    MenuFile,\n    MenuEdit,\n    MenuView,\n    MenuHelp\n};',
         '    MenuFile,\n    MenuEdit,\n    MenuView,\n    MenuHelp,\n    DuplicateSelection,\n    DeleteSelectionSafe\n};',
         'append unified duplicate/delete commands')
    edit('builder_h',
         '    bool BeginCatalogDrag(int filteredIndex);',
         '    bool BeginCatalogDrag(int filteredIndex);\n    bool BeginDuplicateSelectedPlacement();',
         'public staged duplicate API')
    edit('builder_h',
         '    bool RemoveSelectedModule();\n    bool DetachSelectedModule();',
         '    bool RemoveSelectedModule();\n    bool RemoveSelectedModulePreserveDescendants();\n    bool DetachSelectedModule();',
         'safe delete private API')

    edit('builder_cpp',
         '#include "ship_editor/ShipyardBuilderSystem.h"',
         '#include "ship_editor/ShipyardBuilderSystem.h"\n#include "studio/StudioPlacementWorkflowPolicy.h"',
         'placement workflow include')

    # A normal duplicate means one selected object. Live symmetry remains a
    # placement aid for palette placement and explicit mirror workflows, but it
    # must not silently turn Shift+D into two new modules.
    edit('drag_h',
         '    bool mirroredPreviewActive = false;\n    bool mirroredValid = false;',
         '    bool mirroredPreviewActive = false;\n    bool mirroredValid = false;\n    bool suppressLiveSymmetry = false;',
         'duplicate live-symmetry suppression flag')
    edit('builder_cpp',
         '    if(!model_.dragPreview.active||!model_.symmetryFrame.live)return;',
         '    if(!model_.dragPreview.active||!model_.symmetryFrame.live||model_.dragPreview.suppressLiveSymmetry)return;',
         'respect duplicate symmetry suppression')

    # R66: a staged placement already owns pendingDragHistory_. Do not push a
    # second history snapshot when an old AddModule surface is used to confirm.
    edit('builder_cpp',
         '''    const bool record=RecordsAuthoringHistory(command);\n    ShipyardBuilderRuntimeModel before;\n    if(record)before=model_;''',
         '''    const bool deferredDuplicate=command==ShipyardBuilderCommand::DuplicateSelection&&\n        model_.workspaceMode!=ShipyardWorkspaceMode::Model;\n    const bool stagedAddCommit=command==ShipyardBuilderCommand::AddModule&&model_.dragPreview.staged;\n    const bool record=RecordsAuthoringHistory(command)&&!deferredDuplicate&&!stagedAddCommit;\n    ShipyardBuilderRuntimeModel before;\n    if(record)before=model_;''',
         'single placement undo authority')

    # Both unified commands participate in normal history. Construct duplicate
    # is suppressed above until explicit placement confirmation; Geometry is
    # immediate and therefore records here.
    edit('builder_cpp',
         '    case ShipyardBuilderCommand::RemoveModule:\n    case ShipyardBuilderCommand::DetachModule:',
         '    case ShipyardBuilderCommand::RemoveModule:\n    case ShipyardBuilderCommand::DuplicateSelection:\n    case ShipyardBuilderCommand::DeleteSelectionSafe:\n    case ShipyardBuilderCommand::DetachModule:',
         'unified selection history commands')

    # R63: only create pending history after the drag request has been proven
    # valid. Invalid palette indices must not poison the next placement undo.
    edit('builder_cpp',
         '''bool ShipyardBuilderSystem::BeginCatalogDrag(int filteredIndex){\n    if(!pendingDragHistory_)pendingDragHistory_=model_;\n    const auto filtered=FilteredCatalogIndices();if(filteredIndex<0||static_cast<std::size_t>(filteredIndex)>=filtered.size())return false;model_.selectedFilteredModule=static_cast<std::size_t>(filteredIndex);''',
         '''bool ShipyardBuilderSystem::BeginCatalogDrag(int filteredIndex){\n    const auto filtered=FilteredCatalogIndices();if(filteredIndex<0||static_cast<std::size_t>(filteredIndex)>=filtered.size())return false;\n    if(!pendingDragHistory_)pendingDragHistory_=model_;\n    model_.selectedFilteredModule=static_cast<std::size_t>(filteredIndex);''',
         'valid drag owns history')

    # R64: duplicate selected Construct module into the existing staged placement
    # system. Preserve authored transform/material/mirror values and keep all
    # socket candidate information available for later explicit snap selection.
    duplicate_impl=r'''
bool ShipyardBuilderSystem::BeginDuplicateSelectedPlacement(){
    if(model_.workspaceMode==ShipyardWorkspaceMode::Model||model_.recipe.modules.empty()||model_.dragPreview.active)return false;
    const std::size_t index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    const auto* record=FindRecord(model_.recipe.modules[index].moduleId);if(!record)return false;
    if(pendingDragHistory_)return false;
    pendingDragHistory_=model_;
    auto preview=ShipyardDragDropSystem::Begin(*record,model_.catalog,model_.recipe,model_.targetModuleSize);
    preview.ghost=model_.recipe.modules[index];
    preview.ghost.x+=StudioPlacementWorkflowPolicy::DuplicateOffset(model_.transformSnap);
    preview.active=true;preview.valid=true;preview.staged=true;preview.snapped=false;preview.freePlacement=true;preview.selectedCandidate=-1;preview.suppressLiveSymmetry=true;
    preview.resolvedUniformScale=(preview.ghost.scaleX+preview.ghost.scaleY+preview.ghost.scaleZ)/3.0f;
    preview.status="DUPLICATE STAGED / MOVE - ROTATE - SCALE / ENTER CONFIRMS / ESC CANCELS";
    model_.dragPreview=std::move(preview);RefreshDragSymmetryPreview();model_.status=model_.dragPreview.status;return true;
}
'''
    anchor='''bool ShipyardBuilderSystem::UpdateCatalogDrag(const Vector3& shipLocalPointer){'''
    if 'bool ShipyardBuilderSystem::BeginDuplicateSelectedPlacement()' not in changed['builder_cpp']:
        n=changed['builder_cpp'].count(anchor)
        if n!=1:raise RuntimeError(f'duplicate implementation anchor: expected one, found {n}')
        changed['builder_cpp']=changed['builder_cpp'].replace(anchor,duplicate_impl+'\n'+anchor,1);touched.append('builder_cpp') if 'builder_cpp' not in touched else None

    # R67/R69: normal Delete removes one module, preserves descendants as detached
    # drafts, reindexes all surviving graph-owned indices, and keeps selection on
    # the item that shifted into the deleted slot.
    safe_delete=r'''
bool ShipyardBuilderSystem::RemoveSelectedModulePreserveDescendants(){
    if(model_.recipe.modules.empty())return false;
    const std::size_t index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    const std::string id=model_.recipe.modules[index].moduleId;
    std::size_t detachedChildren=0;for(const auto& e:model_.recipe.attachments)if(e.parentModuleIndex==index)++detachedChildren;
    std::vector<std::size_t> remap(model_.recipe.modules.size(),static_cast<std::size_t>(-1));
    std::vector<VisualModulePlacement> kept;kept.reserve(model_.recipe.modules.size()-1);
    for(std::size_t i=0;i<model_.recipe.modules.size();++i)if(i!=index){remap[i]=kept.size();kept.push_back(model_.recipe.modules[i]);}
    std::vector<ShipVisualAttachment> edges;edges.reserve(model_.recipe.attachments.size());
    for(auto e:model_.recipe.attachments){
        if(e.parentModuleIndex==index||e.childModuleIndex==index)continue;
        if(e.parentModuleIndex>=remap.size()||e.childModuleIndex>=remap.size())continue;
        if(remap[e.parentModuleIndex]==static_cast<std::size_t>(-1)||remap[e.childModuleIndex]==static_cast<std::size_t>(-1))continue;
        e.parentModuleIndex=remap[e.parentModuleIndex];e.childModuleIndex=remap[e.childModuleIndex];edges.push_back(std::move(e));
    }
    std::vector<ConstructionSymmetryPair> symmetry;symmetry.reserve(model_.symmetryPairs.size());
    for(auto pair:model_.symmetryPairs){
        if(pair.first==index||pair.second==index)continue;
        if(pair.first>=remap.size()||pair.second>=remap.size())continue;
        if(remap[pair.first]==static_cast<std::size_t>(-1)||remap[pair.second]==static_cast<std::size_t>(-1))continue;
        pair.first=remap[pair.first];pair.second=remap[pair.second];symmetry.push_back(pair);
    }
    model_.recipe.modules=std::move(kept);model_.recipe.attachments=std::move(edges);model_.symmetryPairs=std::move(symmetry);
    ShipArticulationSystem::ReindexAfterModuleRemoval(model_.recipe,remap);
    model_.selectedPlacedModule=StudioPlacementWorkflowPolicy::SelectionAfterRemoval(index,model_.recipe.modules.size());
    RefreshForwardAuthority();InvalidateRecipeMetadata();NormalizeSelections();SyncCatalogSelectionToPlaced();model_.validation=Validate();
    model_.status="Deleted selected "+CompactId(id)+(detachedChildren?" / preserved "+std::to_string(detachedChildren)+" child module(s) as detached drafts":"");return true;
}
'''
    anchor2='''bool ShipyardBuilderSystem::DetachSelectedModule(){'''
    if 'bool ShipyardBuilderSystem::RemoveSelectedModulePreserveDescendants()' not in changed['builder_cpp']:
        n=changed['builder_cpp'].count(anchor2)
        if n!=1:raise RuntimeError(f'safe delete implementation anchor: expected one, found {n}')
        changed['builder_cpp']=changed['builder_cpp'].replace(anchor2,safe_delete+'\n'+anchor2,1);touched.append('builder_cpp') if 'builder_cpp' not in touched else None

    # R70 command convergence: same public actions choose correct document lane.
    edit('builder_cpp',
         '        case ShipyardBuilderCommand::AddModule:changed=model_.dragPreview.staged?CommitCatalogDrag():AddSelectedModule();break;\n        case ShipyardBuilderCommand::ReplaceModule:',
         '''        case ShipyardBuilderCommand::AddModule:changed=model_.dragPreview.staged?CommitCatalogDrag():AddSelectedModule();break;\n        case ShipyardBuilderCommand::DuplicateSelection:{\n            if(model_.workspaceMode==ShipyardWorkspaceMode::Model){if(model_.modeling.recipe.primitives.empty())return false;const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);const bool ok=ShipyardModelingSystem::DuplicatePrimitive(model_.modeling.recipe,index);if(ok){model_.modeling.selectedPrimitiveIndex=model_.modeling.recipe.primitives.size()-1;model_.dirty=true;model_.status="Duplicated modeled shape";}return ok;}\n            return BeginDuplicateSelectedPlacement();}\n        case ShipyardBuilderCommand::DeleteSelectionSafe:{\n            if(model_.workspaceMode==ShipyardWorkspaceMode::Model){if(model_.modeling.recipe.primitives.empty())return false;const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);const bool ok=ShipyardModelingSystem::RemovePrimitive(model_.modeling.recipe,index);if(ok){model_.modeling.selectedPrimitiveIndex=model_.modeling.recipe.primitives.empty()?0:std::min(index,model_.modeling.recipe.primitives.size()-1);model_.dirty=true;model_.status="Deleted selected modeled shape";}return ok;}\n            return RemoveSelectedModulePreserveDescendants();}\n        case ShipyardBuilderCommand::ReplaceModule:''',
         'unified duplicate/delete activation')

    # R65/R66/R68: shared shortcut and context-aware Delete. Enter means confirm
    # while a staged preview exists; it no longer routes through AddModule.
    edit('app_cpp',
         '    if(input_.WasPressed(InputAction::MenuAccept))RouteControl(ShipyardBuilderCommand::AddModule);',
         '    if(input_.WasPressed(InputAction::MenuAccept))RouteControl(builder_.Model().dragPreview.staged?ShipyardBuilderCommand::ConfirmPlacement:ShipyardBuilderCommand::AddModule);',
         'explicit staged Enter confirmation')
    edit('app_cpp',
         '''        if(!ctrl){\n            if(input_.WasPressed(InputAction::DccCycleTransformSpace))RouteControl(ShipyardBuilderCommand::ToggleTransformSpace);\n            if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);''',
         '''        if(!ctrl){\n            if(input_.WasPressed(InputAction::DccCycleTransformSpace))RouteControl(ShipyardBuilderCommand::ToggleTransformSpace);\n            if(input_.WasPressed(InputAction::DccDuplicateSelection))RouteControl(ShipyardBuilderCommand::DuplicateSelection);\n            if(input_.WasPressed(InputAction::EditorToolSelect))RouteControl(ShipyardBuilderCommand::ToolSelect);''',
         'shared duplicate shortcut routing')
    edit('app_cpp',
         '''            if(input_.WasPressed(InputAction::EditorDeleteModule)){\n                switch(builder_.Model().workspaceMode){\n                case ShipyardWorkspaceMode::Build:RouteControl(ShipyardBuilderCommand::RemoveModule);break;\n                case ShipyardWorkspaceMode::Model:RouteControl(ShipyardBuilderCommand::ModelRemovePrimitive);break;\n                case ShipyardWorkspaceMode::Interior:RouteControl(ShipyardBuilderCommand::InteriorRemoveElement);break;\n                default:break;\n                }\n            }''',
         '''            if(input_.WasPressed(InputAction::EditorDeleteModule)){\n                if(builder_.Model().inspectorTab==ShipyardInspectorTab::Sockets)RouteControl(ShipyardBuilderCommand::RemoveSocket);\n                else switch(builder_.Model().workspaceMode){\n                case ShipyardWorkspaceMode::Build:case ShipyardWorkspaceMode::Model:RouteControl(ShipyardBuilderCommand::DeleteSelectionSafe);break;\n                case ShipyardWorkspaceMode::Interior:RouteControl(ShipyardBuilderCommand::InteriorRemoveElement);break;\n                default:break;\n                }\n            }''',
         'context-aware safe Delete')

    # R70 visible Edit/Geometry surfaces use the same public commands. These are
    # tiny guarded substitutions rather than replacing the local visible-shell file.
    edit('visible_cpp',
         '''            menuRow(ShipyardBuilderCommand::UndoAuthoring,"UNDO",true,0);\n            menuRow(ShipyardBuilderCommand::RedoAuthoring,"REDO",true,1);\n            menuRow(ShipyardBuilderCommand::RemoveModule,"DELETE MODULE",HasPlaced(model),2);''',
         '''            menuRow(ShipyardBuilderCommand::UndoAuthoring,"UNDO",true,0);\n            menuRow(ShipyardBuilderCommand::RedoAuthoring,"REDO",true,1);\n            menuRow(ShipyardBuilderCommand::DuplicateSelection,"DUPLICATE",HasPlaced(model)||!model.modeling.recipe.primitives.empty(),2);\n            menuRow(ShipyardBuilderCommand::DeleteSelectionSafe,"DELETE SELECTED",HasPlaced(model)||!model.modeling.recipe.primitives.empty(),3);''',
         'safe Edit menu actions')
    edit('visible_cpp',
         '                    ShipyardBuilderCommand::ModelDuplicatePrimitive,"DUPLICATE",false,hasShape,ay);',
         '                    ShipyardBuilderCommand::DuplicateSelection,"DUPLICATE",false,hasShape,ay);',
         'Geometry duplicate convergence')
    edit('visible_cpp',
         '            add(ShipyardBuilderCommand::ModelRemovePrimitive,0,tx,railButtonsY+5*(th+gap),tw,th,"DEL",false,hasShape);',
         '            add(ShipyardBuilderCommand::DeleteSelectionSafe,0,tx,railButtonsY+5*(th+gap),tw,th,"DEL",false,hasShape);',
         'Geometry delete convergence')
    edit('visible_cpp',
         '''    const bool revealExterior=command==ShipyardBuilderCommand::AddModule||
        command==ShipyardBuilderCommand::ConfirmPlacement||''',
         '''    const bool revealExterior=command==ShipyardBuilderCommand::AddModule||
        command==ShipyardBuilderCommand::ConfirmPlacement||
        command==ShipyardBuilderCommand::DuplicateSelection||''',
         'duplicate preview exterior visibility')

    # Assertions describe the behavior the native build must receive.
    for k,tok,label in [
      ('input_h','DccDuplicateSelection','duplicate input action'),
      ('window_cpp','InputAction::DccDuplicateSelection','Shift+D platform binding'),
      ('builder_h','DuplicateSelection','duplicate public command'),
      ('builder_h','DeleteSelectionSafe','safe delete public command'),
      ('builder_cpp','BeginDuplicateSelectedPlacement()','staged duplicate implementation'),
      ('builder_cpp','RemoveSelectedModulePreserveDescendants()','safe delete implementation'),
      ('builder_cpp','stagedAddCommit','single placement undo'),
      ('drag_h','suppressLiveSymmetry','single-copy duplicate symmetry policy'),
      ('app_cpp','ShipyardBuilderCommand::ConfirmPlacement:ShipyardBuilderCommand::AddModule','Enter commit routing'),
      ('app_cpp','ShipyardInspectorTab::Sockets)RouteControl(ShipyardBuilderCommand::RemoveSocket)','socket delete routing'),
      ('visible_cpp','"DELETE SELECTED"','safe visible Edit menu'),
      ('visible_cpp','command==ShipyardBuilderCommand::DuplicateSelection','duplicate exterior reveal'),
    ]:require(changed[k],tok,label)

    if not args.apply:
        print('R63-R72 DRY RUN PASS; files requiring normalization:',', '.join(touched) if touched else 'none');return 0
    if not touched:
        print('R63-R72 selection/placement ergonomics already present; no mutation required.');return 0
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    backup=root/'artifacts/gates/migrations/studio_r63_r72'/stamp;backup.mkdir(parents=True,exist_ok=False)
    receipt={'schema':'subspace.studio-r63-r72-migration.v1','timestampUtc':stamp,'files':[]}
    for k in touched:
        p=files[k];rel=p.relative_to(root);dst=backup/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dst)
        receipt['files'].append({'path':rel.as_posix(),'beforeSha256':sha256(p.read_bytes())})
    try:
        for k in touched:
            p=files[k];tmp=p.with_name(p.name+'.r63r72.tmp');tmp.write_text(changed[k],encoding='utf-8',newline='\n');os.replace(tmp,p)
        for item in receipt['files']:item['afterSha256']=sha256((root/item['path']).read_bytes())
        (backup/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    except Exception:
        for item in receipt['files']:
            rel=Path(item['path']);src=backup/rel
            if src.exists():shutil.copy2(src,root/rel)
        raise
    print(f'R63-R72 selection/placement ergonomics applied transactionally to {len(touched)} file(s).')
    print(f'Backup/receipt: {backup}')
    return 0

if __name__=='__main__':
    try: raise SystemExit(main())
    except Exception as exc:
        print(f'R63-R72 NORMALIZATION BLOCKED: {exc}',file=sys.stderr);raise SystemExit(2)
