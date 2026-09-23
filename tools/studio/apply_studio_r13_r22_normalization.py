#!/usr/bin/env python3
"""Transactional R13-R22 Studio normalization source migration.

The PCC patch installs the new helpers and this migrator.  Large active source
files are edited only after all expected anchors have been preflighted.  The
migration creates recovery copies and a machine-readable receipt before any
source is replaced.  Unknown source drift fails closed.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime, timezone

MIGRATION_ID = "subspace-studio-r13-r22-normalization-v1"
TARGET_FILES = (
    "engine/src/ship_editor/ShipyardBuilderSystem.cpp",
    "engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp",
    "engine/src/ship_editor/ShipyardWorkspaceSystem.cpp",
    "engine/src/modeling/ShipyardModelingSystem.cpp",
)
EXPECTED_BASELINE_BLOB = {
    TARGET_FILES[0]: "82502626c1659029b96dcf337b6837b0593ea1b2",
    TARGET_FILES[1]: "58b3000b9e8b28bcab8840da50c7463e836ce7a9",
    TARGET_FILES[2]: "48f2cfac28a5369c9d50e7a66c90c91aa75a0a4e",
    TARGET_FILES[3]: "0a3402c2106c36023a72db9ad4b622c7bd611a4c",
}


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_path(path: Path) -> str:
    return sha256_bytes(path.read_bytes())

def git_blob_sha1(data: bytes) -> str:
    header = f"blob {len(data)}\0".encode("ascii")
    return hashlib.sha1(header + data).hexdigest()


def replace_once(text: str, old: str, new: str, label: str) -> tuple[str, str]:
    """Return (new_text,state), where state is APPLY or PRESENT."""
    old_count = text.count(old)
    new_count = text.count(new)
    if old_count == 1 and new_count == 0:
        return text.replace(old, new, 1), "APPLY"
    if old_count == 0 and new_count == 1:
        return text, "PRESENT"
    raise RuntimeError(
        f"{label}: expected exactly one old anchor or one already-applied anchor "
        f"(old={old_count}, new={new_count})"
    )


def transform_builder(text: str) -> tuple[str, list[dict]]:
    log: list[dict] = []
    def edit(old: str, new: str, label: str):
        nonlocal text
        text, state = replace_once(text, old, new, label)
        log.append({"edit": label, "state": state})

    inc = '#include "ship_editor/ShipyardKitbashTransformSystem.h"'
    edit(inc, inc+'\n#include "editor/ConstructionScalePivotSystem.h"', "builder scale-pivot include")

    old_scale = '''bool ShipyardBuilderSystem::ScaleSelected(const Vector3& deltaScale,bool fine){
    if(model_.workspaceMode==ShipyardWorkspaceMode::Model&&!model_.modeling.recipe.primitives.empty()){
        const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);
        Vector3 d=ApplyTransformConstraint(deltaScale,model_.transformConstraint);if(model_.transformConstraint==ShipyardTransformConstraint::Free)d=deltaScale;if(fine)d=d*.1f;
        const bool ok=ShipyardModelingSystem::ScalePrimitive(model_.modeling.recipe,index,d);if(ok){model_.dirty=true;model_.status=std::string("Modeled shape scaled / ")+TransformConstraintName(model_.transformConstraint);}return ok;
    }
    if(!model_.transform.active && !BeginSelectedTransform()) return false;
    if(model_.transform.tool!=ShipyardTransformTool::Scale) return false;
    ShipyardTransformSystem::Scale(model_.transform,ApplyTransformConstraint(deltaScale,model_.transformConstraint),fine);
    if(model_.transform.moduleIndex<model_.recipe.modules.size()){
        if(const auto* record=FindRecord(model_.recipe.modules[model_.transform.moduleIndex].moduleId))
            ClampPlacementToMorphProfile(*record,model_.transform.before,model_.transform.working);
        model_.recipe.modules[model_.transform.moduleIndex]=model_.transform.working;
    }
    model_.dirty=true;InvalidateRecipeMetadata();return true;
}
'''
    new_scale = '''bool ShipyardBuilderSystem::ScaleSelected(const Vector3& deltaScale,bool fine){
    if(model_.workspaceMode==ShipyardWorkspaceMode::Model&&!model_.modeling.recipe.primitives.empty()){
        const auto index=std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1);
        const auto before=model_.modeling.recipe.primitives[index];
        Vector3 d=ApplyTransformConstraint(deltaScale,model_.transformConstraint);if(model_.transformConstraint==ShipyardTransformConstraint::Free)d=deltaScale;if(fine)d=d*.1f;
        const bool anchored=model_.transformConstraint!=ShipyardTransformConstraint::Free;
        const bool ok=ShipyardModelingSystem::ScalePrimitive(model_.modeling.recipe,index,d);
        if(ok){
            if(anchored){
                auto& after=model_.modeling.recipe.primitives[index];
                const auto basis=ConstructionTransformBasisSystem::ModelLocal(before.rotationDegrees);
                const auto shift=ConstructionScalePivotSystem::OppositeFaceShift(before.size,after.size,basis);
                after.position=before.position+shift;
            }
            model_.dirty=true;model_.status=anchored?"Modeled shape scaled / opposite face anchored":std::string("Modeled shape scaled / ")+TransformConstraintName(model_.transformConstraint);
        }
        return ok;
    }
    if(!model_.transform.active && !BeginSelectedTransform()) return false;
    if(model_.transform.tool!=ShipyardTransformTool::Scale) return false;
    ShipyardTransformSystem::Scale(model_.transform,ApplyTransformConstraint(deltaScale,model_.transformConstraint),fine);
    if(model_.transform.moduleIndex<model_.recipe.modules.size()){
        if(const auto* record=FindRecord(model_.recipe.modules[model_.transform.moduleIndex].moduleId)){
            ClampPlacementToMorphProfile(*record,model_.transform.before,model_.transform.working);
            const auto profile=UniversalKitbashAuthority::BuildProfile(*record,KitbashMaterialCertification::NormalizedFallback);
            const bool anchored=model_.transformConstraint!=ShipyardTransformConstraint::Free&&
                profile.morph.policy!=KitbashScalingPolicy::DiscreteFamily&&
                profile.morph.policy!=KitbashScalingPolicy::FixedReference;
            if(anchored){
                const auto position=ConstructionScalePivotSystem::AnchoredModulePosition(record->source,model_.transform.before,model_.transform.working);
                model_.transform.working.x=position.x;model_.transform.working.y=position.y;model_.transform.working.z=position.z;
            }
        }
        model_.recipe.modules[model_.transform.moduleIndex]=model_.transform.working;
    }
    model_.dirty=true;InvalidateRecipeMetadata();return true;
}
'''
    edit(old_scale, new_scale, "opposite-face anchored scale")

    old_tabs='''    workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceBuild,"BUILD",effectiveMode==ShipyardWorkspaceMode::Build,true});
    if(model.capabilities.model)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceModel,"MODEL",effectiveMode==ShipyardWorkspaceMode::Model,true});'''
    new_tabs='''    workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceBuild,"CONSTRUCT",effectiveMode==ShipyardWorkspaceMode::Build||effectiveMode==ShipyardWorkspaceMode::Model,true});'''
    edit(old_tabs,new_tabs,"legacy construct workspace projection")

    edit('{ShipyardBuilderCommand::FrameShip,"[HOME] FRAME SHIP",false,hasPlaced}\n        });',
         '{ShipyardBuilderCommand::FrameShip,"[HOME] FRAME SHIP",false,hasPlaced},\n            {ShipyardBuilderCommand::WorkspaceModel,"EDIT GEOMETRY",false,model.capabilities.model}\n        });',
         "construct geometry entry")
    edit('{ShipyardBuilderCommand::ModelToggleSymmetricStretch,model.modeling.symmetricStretch?"STRETCH SYM":"STRETCH ONE SIDE",model.modeling.symmetricStretch,true}\n        });',
         '{ShipyardBuilderCommand::ModelToggleSymmetricStretch,model.modeling.symmetricStretch?"STRETCH SYM":"STRETCH ONE SIDE",model.modeling.symmetricStretch,true},\n            {ShipyardBuilderCommand::WorkspaceBuild,"ASSEMBLY",false,true}\n        });',
         "geometry assembly return")

    # Generic visible movement/symmetry wording. Command IDs stay compatible.
    for old,new,label in (
        ('{ShipyardBuilderCommand::NudgePort,"PORT",false,hasTransformSubject}', '{ShipyardBuilderCommand::NudgePort,"X -",false,hasTransformSubject}', "nudge x negative"),
        ('{ShipyardBuilderCommand::NudgeStarboard,"STARBOARD",false,hasTransformSubject}', '{ShipyardBuilderCommand::NudgeStarboard,"X +",false,hasTransformSubject}', "nudge x positive"),
        ('{ShipyardBuilderCommand::NudgeForward,"FORWARD",false,hasTransformSubject}', '{ShipyardBuilderCommand::NudgeForward,"Y +",false,hasTransformSubject}', "nudge y positive"),
        ('{ShipyardBuilderCommand::NudgeAft,"AFT",false,hasTransformSubject}', '{ShipyardBuilderCommand::NudgeAft,"Y -",false,hasTransformSubject}', "nudge y negative"),
        ('{ShipyardBuilderCommand::NudgeDorsal,"UP",false,hasTransformSubject}', '{ShipyardBuilderCommand::NudgeDorsal,"Z +",false,hasTransformSubject}', "nudge z positive"),
        ('{ShipyardBuilderCommand::NudgeVentral,"DOWN",false,hasTransformSubject}', '{ShipyardBuilderCommand::NudgeVentral,"Z -",false,hasTransformSubject}', "nudge z negative"),
        ('{ShipyardBuilderCommand::SymmetryAxisPortStarboard,"PORT <-> STARBOARD",model.symmetryFrame.axis==ConstructionSymmetryAxis::PortStarboard,true}', '{ShipyardBuilderCommand::SymmetryAxisPortStarboard,"MIRROR X",model.symmetryFrame.axis==ConstructionSymmetryAxis::PortStarboard,true}', "symmetry X label"),
        ('{ShipyardBuilderCommand::SymmetryAxisForeAft,"FORE <-> AFT",model.symmetryFrame.axis==ConstructionSymmetryAxis::ForeAft,true}', '{ShipyardBuilderCommand::SymmetryAxisForeAft,"MIRROR Y",model.symmetryFrame.axis==ConstructionSymmetryAxis::ForeAft,true}', "symmetry Y label"),
        ('{ShipyardBuilderCommand::SymmetryAxisDorsalVentral,"DORSAL <-> VENTRAL",model.symmetryFrame.axis==ConstructionSymmetryAxis::DorsalVentral,true}', '{ShipyardBuilderCommand::SymmetryAxisDorsalVentral,"MIRROR Z",model.symmetryFrame.axis==ConstructionSymmetryAxis::DorsalVentral,true}', "symmetry Z label"),
    ):
        edit(old,new,label)
    return text, log


def transform_visible(text: str) -> tuple[str, list[dict]]:
    log: list[dict] = []
    def edit(old: str, new: str, label: str):
        nonlocal text
        text, state = replace_once(text, old, new, label)
        log.append({"edit": label, "state": state})

    old_tabs='''    // First-class authoring flow: assembly -> model -> interior -> systems ->
    // paint -> test. Developer-only workspaces remain under DEV.
    float bx=8.0f*s;const float tabW=82.0f*s,tabGap=2.0f*s;
    auto tab=[&](ShipyardBuilderCommand c,const char* label,bool active,bool enabled=true){add(c,0,bx,l.workspaceBarY,tabW,l.workspaceBarHeight,label,active,enabled);bx+=tabW+tabGap;};
    tab(ShipyardBuilderCommand::WorkspaceBuild,"ASSEMBLY",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Build);
    tab(ShipyardBuilderCommand::WorkspaceModel,"MODEL",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Model,model.capabilities.model);'''
    new_tabs='''    // First-class authoring flow: Construct owns assembly + geometry submodes,
    // followed by interior, systems, appearance and test.  Internal Build/Model
    // enum values remain separate only for document/command compatibility.
    float bx=8.0f*s;const float tabW=96.0f*s,tabGap=2.0f*s;
    auto tab=[&](ShipyardBuilderCommand c,const char* label,bool active,bool enabled=true){add(c,0,bx,l.workspaceBarY,tabW,l.workspaceBarHeight,label,active,enabled);bx+=tabW+tabGap;};
    tab(ShipyardBuilderCommand::WorkspaceBuild,"CONSTRUCT",!model.testWorkspaceActive&&(model.workspaceMode==ShipyardWorkspaceMode::Build||model.workspaceMode==ShipyardWorkspaceMode::Model));'''
    edit(old_tabs,new_tabs,"professional Construct tab merge")

    old_model='''            add(ShipyardBuilderCommand::ModelAddBox,0,tx,railButtonsY+4*(th+gap),tw,th,"ADD BOX",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelRemovePrimitive,0,tx,railButtonsY+5*(th+gap),tw,th,"DEL",false,hasShape);'''
    new_model='''            add(ShipyardBuilderCommand::ModelAddBox,0,tx,railButtonsY+4*(th+gap),tw,th,"ADD BOX",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelRemovePrimitive,0,tx,railButtonsY+5*(th+gap),tw,th,"DEL",false,hasShape);
            add(ShipyardBuilderCommand::WorkspaceBuild,0,tx,railButtonsY+6*(th+gap),tw,th,"ASSEMBLY",false,true);'''
    edit(old_model,new_model,"shared rail assembly return")

    old_assembly='''            add(ShipyardBuilderCommand::ToggleTransformSnap,0,tx,railButtonsY+4*(th+gap),tw,th,model.standaloneDesign?(model.transformSnap?"SNAP ON":"SNAP OFF"):"SNAP",model.transformSnap,HasPlaced(model));
            add(ShipyardBuilderCommand::FrameSelected,0,tx,railButtonsY+5*(th+gap),tw,th,model.standaloneDesign?"FRAME":"F",false,HasPlaced(model));'''
    new_assembly='''            add(ShipyardBuilderCommand::ToggleTransformSnap,0,tx,railButtonsY+4*(th+gap),tw,th,model.standaloneDesign?(model.transformSnap?"SNAP ON":"SNAP OFF"):"SNAP",model.transformSnap,HasPlaced(model));
            add(ShipyardBuilderCommand::FrameSelected,0,tx,railButtonsY+5*(th+gap),tw,th,model.standaloneDesign?"FRAME":"F",false,HasPlaced(model));
            if(model.standaloneDesign&&model.capabilities.model)
                add(ShipyardBuilderCommand::WorkspaceModel,0,tx,railButtonsY+6*(th+gap),tw,th,"GEOMETRY",false,true);'''
    edit(old_assembly,new_assembly,"shared rail geometry entry")

    old_search='''        const float hb=std::max(24.0f,22.0f*s),hgap=2.0f*s;
        add(ShipyardBuilderCommand::DccPanelToggleCollapse,0,ax+aw-(hb*4+hgap*3)-5*s,ay+4*s,hb,hb,"-",DockPanelCollapsed(model,"asset_browser"),true);
        add(ShipyardBuilderCommand::DccPanelToggleFloat,0,ax+aw-(hb*3+hgap*2)-5*s,ay+4*s,hb,hb,"[]",false,true);
        add(ShipyardBuilderCommand::DccPanelTogglePin,0,ax+aw-(hb*2+hgap)-5*s,ay+4*s,hb,hb,"P",DockPanelPinned(model,"asset_browser"),true);
        add(ShipyardBuilderCommand::DccPanelToggleVisible,0,ax+aw-hb-5*s,ay+4*s,hb,hb,"X",false,true);
        if(assetBrowserContent){
        // The search field is clickable and keeps native keyboard focus until
        // Enter/Escape or clicking the canvas. It filters the existing catalog.
        if(aw>350*s){
            const float searchW=std::min(230*s,std::max(90*s,aw-205*s));
            add(ShipyardBuilderCommand::DccAssetFocusSearch,0,ax+75*s,ay+4*s,searchW,24*s,
                std::string(model.assetSearchFocused?"SEARCH> ":"SEARCH: ")+
                (model.dcc.assetBrowser.search.empty()?"type to filter":model.dcc.assetBrowser.search),
                model.assetSearchFocused,true);
            if(!model.dcc.assetBrowser.search.empty())
                add(ShipyardBuilderCommand::DccAssetClearSearch,0,ax+75*s+searchW+3*s,ay+4*s,24*s,24*s,"x",false,true);
        }'''
    new_search='''        const float hb=std::max(24.0f,22.0f*s),hgap=2.0f*s;
        const float panelButtonsLeft=ax+aw-(hb*4+hgap*3)-5*s;
        add(ShipyardBuilderCommand::DccPanelToggleCollapse,0,panelButtonsLeft,ay+4*s,hb,hb,"-",DockPanelCollapsed(model,"asset_browser"),true);
        add(ShipyardBuilderCommand::DccPanelToggleFloat,0,panelButtonsLeft+(hb+hgap),ay+4*s,hb,hb,"[]",false,true);
        add(ShipyardBuilderCommand::DccPanelTogglePin,0,panelButtonsLeft+2*(hb+hgap),ay+4*s,hb,hb,"P",DockPanelPinned(model,"asset_browser"),true);
        add(ShipyardBuilderCommand::DccPanelToggleVisible,0,panelButtonsLeft+3*(hb+hgap),ay+4*s,hb,hb,"X",false,true);
        if(assetBrowserContent){
        // Reserve the panel-management buttons before sizing search.  The old
        // aw-205 formula overlapped the clear-X with Collapse at mid widths.
        const float searchX=ax+75*s;
        const float searchAvailable=panelButtonsLeft-searchX-30*s;
        if(searchAvailable>=90*s){
            const float searchW=std::min(230*s,searchAvailable);
            add(ShipyardBuilderCommand::DccAssetFocusSearch,0,searchX,ay+4*s,searchW,24*s,
                std::string(model.assetSearchFocused?"SEARCH> ":"SEARCH: ")+
                (model.dcc.assetBrowser.search.empty()?"type to filter":model.dcc.assetBrowser.search),
                model.assetSearchFocused,true);
            if(!model.dcc.assetBrowser.search.empty())
                add(ShipyardBuilderCommand::DccAssetClearSearch,0,searchX+searchW+3*s,ay+4*s,24*s,24*s,"x",false,true);
        }'''
    edit(old_search,new_search,"asset header overlap repair")
    return text, log


def transform_workspace(text: str) -> tuple[str, list[dict]]:
    log: list[dict] = []
    def edit(old: str, new: str, label: str):
        nonlocal text
        text, state = replace_once(text, old, new, label)
        log.append({"edit": label, "state": state})

    edit('ShipyardWorkspaceSystem::PrimaryWorkspaces(){return {ShipyardWorkspaceMode::Build,ShipyardWorkspaceMode::Model,ShipyardWorkspaceMode::Interior,ShipyardWorkspaceMode::Systems,ShipyardWorkspaceMode::Appearance,ShipyardWorkspaceMode::Test};}',
         'ShipyardWorkspaceSystem::PrimaryWorkspaces(){return {ShipyardWorkspaceMode::Build,ShipyardWorkspaceMode::Interior,ShipyardWorkspaceMode::Systems,ShipyardWorkspaceMode::Appearance,ShipyardWorkspaceMode::Test};}',
         "primary workspace merge")
    edit('case ShipyardWorkspaceMode::Build:return"BUILD";', 'case ShipyardWorkspaceMode::Build:return"CONSTRUCT";', "construct name")
    edit('case ShipyardWorkspaceMode::Model:return"MODEL";', 'case ShipyardWorkspaceMode::Model:return"GEOMETRY";', "geometry submode name")
    edit('add("tool_rail","Tools","tool_left",true,1.0f,48,420,true,true,true);',
         'add("tool_rail","Tools","tool_left",true,1.0f,48,420,false,false,false);',
         "lock shell tool rail")
    edit('h.Register({"shipyard.model","Model","Native geometry creation and region articulation.","Add parametric shapes or select a sub-region for doors, ramps, antennas and other moving parts.",""});',
         'h.Register({"shipyard.model","Construct / Geometry","Native geometry creation inside the Construct workspace.","Switch between Assembly and Geometry without leaving the authored object; both use the same selection and transform tools.",""});',
         "geometry help normalization")
    return text, log


def transform_modeling(text: str) -> tuple[str, list[dict]]:
    log: list[dict] = []
    def edit(old: str, new: str, label: str):
        nonlocal text
        text, state = replace_once(text, old, new, label)
        log.append({"edit": label, "state": state})

    edit('''    // A useful ship-authoring wedge: full-height aft edge (+Y), tapered to the
    // centerline at the forward edge (-Y).
    const Vector3 a{-x,-y,0},b{x,-y,0},c{x,y,-z},dd{-x,y,-z},e{x,y,z},f{-x,y,z};''',
         '''    // Coordinate contract is context-neutral +Y forward.  Keep the full-height
    // aft edge on -Y and taper toward the forward edge on +Y.
    const Vector3 a{-x,y,0},b{x,y,0},c{x,-y,-z},dd{-x,-y,-z},e{x,-y,z},f{-x,-y,z};''',
         "wedge +Y-forward repair")
    return text, log


TRANSFORMS = {
    TARGET_FILES[0]: transform_builder,
    TARGET_FILES[1]: transform_visible,
    TARGET_FILES[2]: transform_workspace,
    TARGET_FILES[3]: transform_modeling,
}


def git_head(root: Path) -> str | None:
    try:
        return subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True, stderr=subprocess.DEVNULL).strip()
    except Exception:
        return None


def prepare(root: Path):
    prepared = {}
    edits = []
    for rel, fn in TRANSFORMS.items():
        path = root / rel
        if not path.is_file():
            raise RuntimeError(f"missing source file: {rel}")
        raw = path.read_bytes()
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError as exc:
            raise RuntimeError(f"{rel}: not UTF-8") from exc
        new_text, log = fn(text)
        changed = new_text != text
        if changed:
            actual_blob = git_blob_sha1(raw)
            expected_blob = EXPECTED_BASELINE_BLOB[rel]
            if actual_blob != expected_blob:
                raise RuntimeError(
                    f"{rel}: source preimage drift; expected git blob {expected_blob}, got {actual_blob}. "
                    "Do not force this migration over local edits or a newer baseline."
                )
        prepared[rel] = {
            "before": raw,
            "after": new_text.encode("utf-8"),
            "changed": changed,
            "beforeGitBlob": git_blob_sha1(raw),
            "beforeSha256": sha256_bytes(raw),
            "afterSha256": sha256_bytes(new_text.encode("utf-8")),
        }
        edits.extend({"file": rel, **e} for e in log)
    return prepared, edits


def write_atomic(path: Path, data: bytes):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(prefix=path.name+".", suffix=".r22.tmp", dir=str(path.parent))
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data); f.flush(); os.fsync(f.fileno())
        os.replace(tmp_name, path)
    finally:
        if os.path.exists(tmp_name):
            os.unlink(tmp_name)


def apply(root: Path) -> Path | None:
    prepared, edits = prepare(root)
    changed = [rel for rel, item in prepared.items() if item["changed"]]
    if not changed:
        print("R13-R22 source migration already present; no writes required.")
        return None

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    recovery = root / ".subspace" / "recovery" / f"studio-r22-{stamp}"
    receipts = root / ".subspace" / "receipts"
    recovery.mkdir(parents=True, exist_ok=False)
    receipts.mkdir(parents=True, exist_ok=True)

    files = []
    try:
        for rel in changed:
            src = root / rel
            dst = recovery / rel
            dst.parent.mkdir(parents=True, exist_ok=True)
            dst.write_bytes(prepared[rel]["before"])
        for rel in changed:
            write_atomic(root / rel, prepared[rel]["after"])
        for rel in changed:
            actual = sha256_path(root / rel)
            if actual != prepared[rel]["afterSha256"]:
                raise RuntimeError(f"post-write hash mismatch: {rel}")
            files.append({
                "path": rel,
                "beforeSha256": prepared[rel]["beforeSha256"],
                "afterSha256": prepared[rel]["afterSha256"],
                "backup": str((recovery / rel).relative_to(root)).replace('\\','/'),
            })
    except Exception:
        for rel in changed:
            backup = recovery / rel
            if backup.is_file():
                write_atomic(root / rel, backup.read_bytes())
        raise

    receipt = {
        "schema": "subspace.studio-source-migration.v1",
        "migrationId": MIGRATION_ID,
        "createdUtc": datetime.now(timezone.utc).isoformat(),
        "gitHead": git_head(root),
        "recoveryRoot": str(recovery.relative_to(root)).replace('\\','/'),
        "files": files,
        "edits": edits,
        "status": "APPLIED",
    }
    receipt_path = receipts / f"studio-r22-{stamp}.json"
    write_atomic(receipt_path, (json.dumps(receipt, indent=2, sort_keys=True)+"\n").encode())
    print(f"R13-R22 source migration APPLIED: {len(files)} file(s)")
    print(f"Receipt: {receipt_path}")
    return receipt_path


def rollback(root: Path, receipt_path: Path):
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    if receipt.get("migrationId") != MIGRATION_ID:
        raise RuntimeError("receipt is not an R13-R22 migration receipt")
    for item in receipt.get("files", []):
        target = root / item["path"]
        backup = root / item["backup"]
        if not backup.is_file():
            raise RuntimeError(f"missing backup: {backup}")
        current = sha256_path(target) if target.is_file() else None
        if current != item["afterSha256"]:
            raise RuntimeError(f"rollback blocked by post-migration edits: {item['path']}")
    for item in receipt.get("files", []):
        write_atomic(root / item["path"], (root / item["backup"]).read_bytes())
    print("R13-R22 rollback complete. Recovery evidence was retained.")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", type=Path, default=Path.cwd())
    group = ap.add_mutually_exclusive_group(required=True)
    group.add_argument("--check", action="store_true")
    group.add_argument("--apply", action="store_true")
    group.add_argument("--rollback", type=Path)
    args = ap.parse_args()
    root = args.root.resolve()
    try:
        if args.rollback:
            rollback(root, args.rollback.resolve()); return 0
        prepared, edits = prepare(root)
        pending = [k for k,v in prepared.items() if v["changed"]]
        if args.check:
            print(f"R13-R22 preflight PASS: {len(edits)} contract edit(s); {len(pending)} file(s) pending")
            for rel in pending: print(f"  PENDING {rel}")
            return 0
        apply(root); return 0
    except Exception as exc:
        print(f"R13-R22 migration BLOCKED: {exc}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())
