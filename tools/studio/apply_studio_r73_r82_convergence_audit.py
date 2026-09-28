#!/usr/bin/env python3
"""Guarded R73-R82 Studio convergence-audit normalization.

Requires R63-R72. Fixes duplicate->snap authored-trait loss and reconciles
Geometry's visible wording with the source that is actually present: live model
preview exists, while catalog publication remains pending. Exact-preimage,
idempotent, transactional, with backup/receipt and fail-closed source drift.
"""
from __future__ import annotations
import argparse, hashlib, json, os, shutil, sys
from datetime import datetime, timezone
from pathlib import Path
from studio_verified_normalized_state import verify_complete_r82r1


def sha256(data: bytes)->str: return hashlib.sha256(data).hexdigest()
def require(text:str,token:str,label:str):
    if token not in text: raise RuntimeError(f"{label}: required token missing: {token}")
def replace_exact(text:str,old:str,new:str,label:str):
    if new in text: return text,False
    n=text.count(old)
    if n!=1: raise RuntimeError(f"{label}: expected one approved preimage, found {n}")
    return text.replace(old,new,1),True

def main()->int:
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);ap.add_argument('--apply',action='store_true');args=ap.parse_args()
    root=Path(args.root).resolve()
    # An R53+ migration extends R33 postimages; do not replay obsolete exact
    # preimages if the full R82R1 state is independently source-gate proven.
    if verify_complete_r82r1(root):
        print("r73_r82_convergence_audit: complete R82R1 normalized source verified; historical migration no-op.")
        return 0
    files={
      'builder_cpp':root/'engine/src/ship_editor/ShipyardBuilderSystem.cpp',
      'renderer_cpp':root/'engine/src/application/NativeBattlefieldRenderer.cpp',
      'visible_cpp':root/'engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp',
      'policy_h':root/'engine/include/studio/StudioPlacementWorkflowPolicy.h',
    }
    for k,p in files.items():
        if not p.is_file(): raise RuntimeError(f"R73-R82 missing source {k}: {p}")
    original={k:p.read_text(encoding='utf-8') for k,p in files.items()}
    require(original['builder_cpp'],'BeginDuplicateSelectedPlacement()','R63-R72 duplicate prerequisite')
    require(original['builder_cpp'],'RemoveSelectedModulePreserveDescendants()','R63-R72 safe-delete prerequisite')
    require(original['builder_cpp'],'suppressLiveSymmetry','R63-R72 duplicate marker prerequisite')
    require(original['policy_h'],'PreserveDuplicateAuthoredTraits','R73-R82 policy payload')
    require(original['renderer_cpp'],'DrawStudioModelScene','Geometry live-preview prerequisite')
    require(original['builder_cpp'],'save/persistence bridge follows','honest canonical-bake prerequisite')

    changed=dict(original);touched=[]
    def edit(k,old,new,label):
        nonlocal changed,touched
        changed[k],did=replace_exact(changed[k],old,new,label)
        if did and k not in touched:touched.append(k)

    # R73/R74: a staged duplicate may pick a socket candidate after preserving
    # a user's authored material/mirrors/non-uniform scale. Candidate selection
    # owns position/orientation only; it must not silently revert those traits
    # to catalog defaults. Apply the rule to explicit candidate cycling and to
    # proximity snapping so future pointer routing cannot reintroduce the loss.
    edit('builder_cpp',
'''bool ShipyardBuilderSystem::CycleStagedSnapCandidate(int delta){
    if(!ShipyardDragDropSystem::CycleCandidate(model_.dragPreview,delta))return false;
    RefreshDragSymmetryPreview();model_.status=model_.dragPreview.status;return true;
}''',
'''bool ShipyardBuilderSystem::CycleStagedSnapCandidate(int delta){
    const bool preserveDuplicateTraits=model_.dragPreview.suppressLiveSymmetry;
    const auto authoredBeforeSnap=model_.dragPreview.ghost;
    if(!ShipyardDragDropSystem::CycleCandidate(model_.dragPreview,delta))return false;
    if(preserveDuplicateTraits)
        StudioPlacementWorkflowPolicy::PreserveDuplicateAuthoredTraits(authoredBeforeSnap,model_.dragPreview.ghost);
    RefreshDragSymmetryPreview();model_.status=model_.dragPreview.status;return true;
}''',
         'duplicate candidate-cycle trait preservation')

    edit('builder_cpp',
'''    if(canSnap){
        model_.dragPreview.selectedCandidate=bestIndex;
        model_.dragPreview.ghost=model_.dragPreview.candidates[static_cast<std::size_t>(bestIndex)].placement;
        model_.dragPreview.valid=!model_.dragPreview.candidates[static_cast<std::size_t>(bestIndex)].collisionRisk;''',
'''    if(canSnap){
        const bool preserveDuplicateTraits=model_.dragPreview.suppressLiveSymmetry;
        const auto authoredBeforeSnap=model_.dragPreview.ghost;
        model_.dragPreview.selectedCandidate=bestIndex;
        model_.dragPreview.ghost=model_.dragPreview.candidates[static_cast<std::size_t>(bestIndex)].placement;
        if(preserveDuplicateTraits)
            StudioPlacementWorkflowPolicy::PreserveDuplicateAuthoredTraits(authoredBeforeSnap,model_.dragPreview.ghost);
        model_.dragPreview.valid=!model_.dragPreview.candidates[static_cast<std::size_t>(bestIndex)].collisionRisk;''',
         'duplicate pointer-snap trait preservation')

    # R78: command-level context safety. Keyboard routing was already context
    # aware in R63-R72, but menus call the command directly; therefore the
    # command itself must never delete/duplicate the wrong document kind.
    edit('builder_cpp',
'''        case ShipyardBuilderCommand::DuplicateSelection:{
            if(model_.workspaceMode==ShipyardWorkspaceMode::Model){if(model_.modeling.recipe.primitives.empty())return false;const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);const bool ok=ShipyardModelingSystem::DuplicatePrimitive(model_.modeling.recipe,index);if(ok){model_.modeling.selectedPrimitiveIndex=model_.modeling.recipe.primitives.size()-1;model_.dirty=true;model_.status="Duplicated modeled shape";}return ok;}
            return BeginDuplicateSelectedPlacement();}
        case ShipyardBuilderCommand::DeleteSelectionSafe:{
            if(model_.workspaceMode==ShipyardWorkspaceMode::Model){if(model_.modeling.recipe.primitives.empty())return false;const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);const bool ok=ShipyardModelingSystem::RemovePrimitive(model_.modeling.recipe,index);if(ok){model_.modeling.selectedPrimitiveIndex=model_.modeling.recipe.primitives.empty()?0:std::min(index,model_.modeling.recipe.primitives.size()-1);model_.dirty=true;model_.status="Deleted selected modeled shape";}return ok;}
            return RemoveSelectedModulePreserveDescendants();}''',
'''        case ShipyardBuilderCommand::DuplicateSelection:{
            if(model_.inspectorTab==ShipyardInspectorTab::Sockets){model_.status="Duplicate is unavailable in SOCKETS; return to CONSTRUCT to duplicate the module";return false;}
            if(model_.workspaceMode==ShipyardWorkspaceMode::Model){if(model_.modeling.recipe.primitives.empty())return false;const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);const bool ok=ShipyardModelingSystem::DuplicatePrimitive(model_.modeling.recipe,index);if(ok){model_.modeling.selectedPrimitiveIndex=model_.modeling.recipe.primitives.size()-1;model_.dirty=true;model_.status="Duplicated modeled shape";}return ok;}
            if(model_.workspaceMode!=ShipyardWorkspaceMode::Build){model_.status="Duplicate is available in CONSTRUCT or GEOMETRY";return false;}
            return BeginDuplicateSelectedPlacement();}
        case ShipyardBuilderCommand::DeleteSelectionSafe:{
            if(model_.inspectorTab==ShipyardInspectorTab::Sockets)return RemoveSocket();
            if(model_.workspaceMode==ShipyardWorkspaceMode::Interior){if(ShipInteriorStructureAuthoringSystem::RemoveSelected(model_.interiorStructure)){model_.dirty=true;model_.status=model_.interiorStructure.status;return true;}return false;}
            if(model_.workspaceMode==ShipyardWorkspaceMode::Model){if(model_.modeling.recipe.primitives.empty())return false;const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);const bool ok=ShipyardModelingSystem::RemovePrimitive(model_.modeling.recipe,index);if(ok){model_.modeling.selectedPrimitiveIndex=model_.modeling.recipe.primitives.empty()?0:std::min(index,model_.modeling.recipe.primitives.size()-1);model_.dirty=true;model_.status="Deleted selected modeled shape";}return ok;}
            if(model_.workspaceMode!=ShipyardWorkspaceMode::Build){model_.status="Delete Selected has no destructive target in this workspace";return false;}
            return RemoveSelectedModulePreserveDescendants();}''',
         'command-level context-safe duplicate/delete')

    edit('visible_cpp',
'''        }else if(model.openMenu==1){
            menuRow(ShipyardBuilderCommand::UndoAuthoring,"UNDO",true,0);
            menuRow(ShipyardBuilderCommand::RedoAuthoring,"REDO",true,1);
            menuRow(ShipyardBuilderCommand::DuplicateSelection,"DUPLICATE",HasPlaced(model)||!model.modeling.recipe.primitives.empty(),2);
            menuRow(ShipyardBuilderCommand::DeleteSelectionSafe,"DELETE SELECTED",HasPlaced(model)||!model.modeling.recipe.primitives.empty(),3);''',
'''        }else if(model.openMenu==1){
            const bool geometrySelection=model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.modeling.recipe.primitives.empty();
            const bool socketSelection=model.inspectorTab==ShipyardInspectorTab::Sockets&&HasPlaced(model);
            const bool interiorSelection=model.workspaceMode==ShipyardWorkspaceMode::Interior&&!model.interiorStructure.elements.empty();
            const bool constructSelection=model.workspaceMode==ShipyardWorkspaceMode::Build&&model.inspectorTab!=ShipyardInspectorTab::Sockets&&HasPlaced(model);
            menuRow(ShipyardBuilderCommand::UndoAuthoring,"UNDO",true,0);
            menuRow(ShipyardBuilderCommand::RedoAuthoring,"REDO",true,1);
            menuRow(ShipyardBuilderCommand::DuplicateSelection,"DUPLICATE",geometrySelection||constructSelection,2);
            menuRow(ShipyardBuilderCommand::DeleteSelectionSafe,"DELETE SELECTED",geometrySelection||socketSelection||interiorSelection||constructSelection,3);''',
         'context-safe Edit menu enablement')

    # R75-R77: source already renders the modeled CanonicalAsset preview. Remove
    # stale text that says preview is unwired. Publication is still NOT closed,
    # so expose the existing in-memory operation as BAKE DRAFT, not Publish.
    edit('renderer_cpp','section("MODEL DRAFT",propertyTextX,layout.editLabelY,propertyWidth);',
         'section("GEOMETRY DRAFT",propertyTextX,layout.editLabelY,propertyWidth);',
         'Geometry section truth')
    edit('renderer_cpp','ShipyardText("Shape recipe only - 3D preview / publish not wired",',
         'ShipyardText("Live geometry preview - catalog publish pending",',
         'Geometry preview truth')
    edit('renderer_cpp',
         '"MODEL DRAFT: Add Box/Wedge/Door on left. Shape preview and publish are not yet wired.":',
         '"GEOMETRY: live canonical preview. Validate or BAKE DRAFT; catalog publication remains pending.":',
         'Geometry status-bar truth')
    edit('visible_cpp',
         'ShipyardBuilderCommand::ModelPublishCanonical,"PUBLISH UNWIRED",false,false,ay);',
         'ShipyardBuilderCommand::ModelPublishCanonical,"BAKE DRAFT",false,hasShape&&model.capabilities.publishCanonicalAsset,ay);',
         'honest enabled Geometry bake action')

    for k,tok,label in [
      ('builder_cpp','PreserveDuplicateAuthoredTraits(authoredBeforeSnap,model_.dragPreview.ghost)','duplicate snap integrity'),
      ('renderer_cpp','GEOMETRY DRAFT','Geometry naming'),
      ('renderer_cpp','Live geometry preview - catalog publish pending','live-preview truth'),
      ('renderer_cpp','BAKE DRAFT; catalog publication remains pending','publication limitation'),
      ('visible_cpp','"BAKE DRAFT"','honest visible bake action'),
      ('builder_cpp','Duplicate is unavailable in SOCKETS','context-safe duplicate command'),
      ('builder_cpp','if(model_.inspectorTab==ShipyardInspectorTab::Sockets)return RemoveSocket();','context-safe delete command'),
      ('visible_cpp','const bool socketSelection=','context-safe Edit menu'),
      ('builder_cpp','save/persistence bridge follows','in-memory bake limitation retained'),
    ]: require(changed[k],tok,label)
    for k,tok,label in [
      ('renderer_cpp','Shape preview and publish are not yet wired','stale preview claim'),
      ('renderer_cpp','3D preview / publish not wired','stale preview/publish claim'),
      ('visible_cpp','PUBLISH UNWIRED','stale publish label'),
    ]:
        if tok in changed[k]: raise RuntimeError(f"{label}: forbidden stale token remains: {tok}")

    if not args.apply:
        print('R73-R82 DRY RUN PASS; files requiring normalization:',', '.join(touched) if touched else 'none');return 0
    if not touched:
        print('R73-R82 convergence audit normalization already present; no mutation required.');return 0
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    backup=root/'artifacts/gates/migrations/studio_r73_r82'/stamp;backup.mkdir(parents=True,exist_ok=False)
    receipt={'schema':'subspace.studio-r73-r82-migration.v1','timestampUtc':stamp,'files':[]}
    for k in touched:
        p=files[k];rel=p.relative_to(root);dst=backup/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dst)
        receipt['files'].append({'path':rel.as_posix(),'beforeSha256':sha256(p.read_bytes())})
    try:
        for k in touched:
            p=files[k];tmp=p.with_name(p.name+'.r73r82.tmp');tmp.write_text(changed[k],encoding='utf-8',newline='\n');os.replace(tmp,p)
        for item in receipt['files']:item['afterSha256']=sha256((root/item['path']).read_bytes())
        (backup/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    except Exception:
        for item in receipt['files']:
            rel=Path(item['path']);src=backup/rel
            if src.exists():shutil.copy2(src,root/rel)
        raise
    print(f'R73-R82 convergence audit normalization applied transactionally to {len(touched)} file(s).')
    print(f'Backup/receipt: {backup}')
    return 0

if __name__=='__main__':
    try: raise SystemExit(main())
    except Exception as exc:
        print(f'R73-R82 NORMALIZATION BLOCKED: {exc}',file=sys.stderr);raise SystemExit(2)
